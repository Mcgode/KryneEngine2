/**
 * @file
 * @author Max Godefroy
 * @date 02/02/2024.
 */

#pragma once

#include "KryneEngine/Core/Memory/GenerationalPool.hpp"

#include "KryneEngine/Core/Common/Assert.hpp"
#include "KryneEngine/Core/Common/Utils/Alignment.hpp"

namespace KryneEngine
{
    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::GenerationalPool(const Allocator& _allocator)
        : m_allocator(_allocator)
        , m_availableIndices(_allocator)
        , m_availableIndicesDeferred(_allocator)
    {
        const auto lock = m_lock.AutoLock();
        _Grow(0);
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::~GenerationalPool()
    {
        for (auto i = 0; i < kSegmentCount; i++)
        {
            Segment segment = m_segments[i].load(std::memory_order::relaxed);
            if (segment != nullptr)
                m_allocator.deallocate(segment);
        }
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    void GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::_Grow(size_t _index)
    {
        KE_ASSERT_MSG(_index < kSegmentCount,
                      "Generational pool maximum growable size is %ull. Consider changing GenPool::IndexType to a bigger type.",
                      kMaxSize);

        const size_t count = 1 << (kInitialSizePot + _index);
        size_t allocationSize = sizeof(HotData) * count;
        size_t alignment = alignof(HotData);
        if constexpr (kHasColdData)
        {
            allocationSize = Alignment::AlignUp(allocationSize, alignof(ColdDataStruct)) + sizeof(ColdDataStruct) * count;
            alignment = eastl::max(alignment, alignof(ColdDataStruct));
        }

        auto segment = static_cast<Segment>(m_allocator.allocate(allocationSize, alignment));
        memset(segment, 0, allocationSize);

        m_segments[_index].store(segment, std::memory_order::release);
    }

    template <class HotDataStruct, class ColdDataStruct, class Allocator>
    size_t GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::GetSegmentIndex(size_t _index)
    {
        return BitUtils::GetMostSignificantBit((_index + kInitialSize) >> kInitialSizePot);
    }

    template <class HotDataStruct, class ColdDataStruct, class Allocator>
    size_t GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::GetLocalIndex(size_t _index, size_t _segmentIndex)
    {
        return _index - ((1 << (_segmentIndex + kInitialSizePot)) - kInitialSize);
    }

    template <class HotDataStruct, class ColdDataStruct, class Allocator>
    ColdDataStruct* GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::GetColdData(Segment _segment, size_t _segmentIndex)
        requires kHasColdData
    {
        const size_t offset = Alignment::AlignUp(
            sizeof(HotData) * (1 << (_segmentIndex + kInitialSizePot)),
            alignof(ColdDataStruct));
        return reinterpret_cast<ColdDataStruct*>(reinterpret_cast<std::byte*>(_segment) + offset);
    }

    template <class HotDataStruct, class ColdDataStruct, class Allocator>
    HotDataStruct* GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::GetHotData(
        Segment _segment,
        size_t _localIndex,
        u32 _generation)
    {
        HotData& hotData = _segment[_localIndex];
        if (std::atomic_ref(hotData.m_generation).load(std::memory_order::acquire) != _generation)
        {
            return nullptr;
        }
        if constexpr (GenPool::IsValidIntrusiveGeneration<HotDataStruct>)
            return &hotData;
        else
            return &hotData.m_userHotData;
    }

    template <class HotDataStruct, class ColdDataStruct, class Allocator>
    HotDataStruct *GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::Get(const GenPool::Handle _handle) const
    {
        VERIFY_OR_RETURN(_handle.m_index < m_size.load(std::memory_order::relaxed), nullptr);

        const size_t segmentIndex = GetSegmentIndex(_handle.m_index);
        const size_t localIndex = GetLocalIndex(_handle.m_index, segmentIndex);
        Segment segment = m_segments[segmentIndex].load(std::memory_order::acquire);
        return GetHotData(segment, localIndex, _handle.m_generation);
    }

    template <class HotDataStruct, class ColdDataStruct, class Allocator>
    eastl::pair<HotDataStruct *, ColdDataStruct *> GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::GetAll(const GenPool::Handle _handle) const
    {
        VERIFY_OR_RETURN(_handle.m_index < m_size.load(std::memory_order::relaxed), (eastl::pair<HotDataStruct *, ColdDataStruct *>{ nullptr, nullptr }));

        const size_t segmentIndex = GetSegmentIndex(_handle.m_index);
        const size_t localIndex = GetLocalIndex(_handle.m_index, segmentIndex);
        Segment segment = m_segments[segmentIndex].load(std::memory_order::acquire);
        HotDataStruct* hotData = GetHotData(segment, localIndex, _handle.m_generation);
        if (hotData == nullptr)
        {
            return eastl::pair<HotDataStruct *, ColdDataStruct *>(nullptr, nullptr);
        }
        if constexpr (kHasColdData)
        {
            return eastl::pair<HotDataStruct*, ColdDataStruct*>(hotData, &GetColdData(segment, segmentIndex)[localIndex]);
        }
        else
        {
            return { hotData, nullptr };
        }
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    GenPool::Handle GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::Allocate()
    {
        const auto lock = m_lock.AutoLock();

        u32 index;
        size_t segmentIndex;
        if (!m_availableIndices.empty())
        {
            // Pop the top of the index stack
            index = m_availableIndices.back();
            m_availableIndices.pop_back();
            segmentIndex = GetSegmentIndex(index);
        }
        else
        {
            index = m_size.fetch_add(1, std::memory_order::acquire);
            VERIFY_OR_RETURN(index < kMaxSize, GenPool::kInvalidHandle);

            segmentIndex = GetSegmentIndex(index);
            if (m_segments[segmentIndex].load(std::memory_order::acquire) == nullptr)
                _Grow(segmentIndex);
        }

        const size_t localIndex = GetLocalIndex(index, segmentIndex);
        Segment segment = m_segments[segmentIndex].load(std::memory_order::acquire);

        return { index, std::atomic_ref(segment[localIndex].m_generation).load(std::memory_order::relaxed) };
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    bool GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::Free(const GenPool::Handle &_handle,
                                                               HotDataStruct *_hotCopy,
                                                               ColdDataStruct *_coldCopy)
    {
        const auto lock = m_lock.AutoLock();

        VERIFY_OR_RETURN(_handle.m_index < m_size.load(std::memory_order::acquire), false);

        const size_t segmentIndex = GetSegmentIndex(_handle.m_index);
        const size_t localIndex = GetLocalIndex(_handle.m_index, segmentIndex);
        Segment segment = m_segments[segmentIndex].load(std::memory_order::acquire);

        auto& hotData = segment[localIndex];
        u32 expectedGeneration = _handle.m_generation;
        const u32 next = (expectedGeneration + 1) % (1 << (32 - GenPool::kIndexBits));
        if (!std::atomic_ref(hotData.m_generation).compare_exchange_strong(expectedGeneration, next, std::memory_order::acq_rel))
        {
            return false;
        }

        if (_hotCopy != nullptr)
        {
            if constexpr (GenPool::IsValidIntrusiveGeneration<HotDataStruct>)
                *_hotCopy = hotData;
            else
                *_hotCopy = hotData.m_userHotData;
        }

        if constexpr (kHasColdData)
        {
            if (_coldCopy != nullptr)
            {
                *_coldCopy = GetColdData(segment, segmentIndex)[localIndex];
            }
        }

        // Place back to the top of the index stack
        m_availableIndicesDeferred.push_back(_handle.m_index);

        return true;
    }

    template <class HotDataStruct, class ColdDataStruct, class Allocator>
    void GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::FlushDeferredFrees()
    {
        const auto lock = m_lock.AutoLock();
        m_availableIndices.insert(m_availableIndices.end(), m_availableIndicesDeferred.begin(), m_availableIndicesDeferred.end());
        m_availableIndicesDeferred.clear();
    }
} // KryneEngine