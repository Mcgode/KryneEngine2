/**
* @file
* @author Max Godefroy
* @date 13/11/2024.
*/

#include "Memory/SimplePool.hpp"

#include <Common/Assert.hpp>

namespace KryneEngine
{
    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::SimplePool()
        : SimplePool(EASTLAllocatorDefault)
    {}

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::SimplePool(const Allocator& _allocator)
        : SimplePool(_allocator, kDefaultPoolSize)
    {}

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::SimplePool(
        const Allocator& _allocator,
        size_t _initialSize)
            : m_allocator(_allocator)
    {
        Resize(_initialSize);
    }

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::~SimplePool()
    {
        if (m_size == 0)
        {
            return;
        }

        m_allocator.deallocate(m_hotData, m_size * sizeof(HotDataItem));

        if constexpr (kHasColdData)
        {
            m_allocator.deallocate(m_coldData, m_size * sizeof(ColdDataStruct));
        }

        if constexpr (RefCounting)
        {
            m_allocator.deallocate(m_refCounts, m_size * sizeof(*m_refCounts));
        }
    }

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    SimplePoolHandle SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::Allocate()
    {
        if (m_size == m_nextFreeIndex)
        {
            Resize(m_size * 2);
        }

        const SimplePoolHandle result = m_nextFreeIndex;
        m_nextFreeIndex = m_hotData[result].m_nextFreeIndex;

        if (RefCounting)
        {
            KE_ASSERT(m_refCounts[result].load(std::memory_order::relaxed) <= 0);
            m_refCounts[result].store(1, std::memory_order::seq_cst);
        }

        return result;
    }

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    bool SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::Free(
        SimplePoolHandle _handle,
        HotDataStruct* _hotCopy,
        ColdDataStruct* _coldDataCopy)
    {
        VERIFY_OR_RETURN(m_size > _handle, false);

        if (_hotCopy != nullptr)
        {
            *_hotCopy = m_hotData[_handle].m_hotData;
        }

        if constexpr (kHasColdData)
        {
            if (_coldDataCopy != nullptr)
            {
                *_coldDataCopy = m_coldData[_handle];
            }
        }

        bool reset = true;

        if constexpr (RefCounting)
        {
            s32 count = m_refCounts[_handle].fetch_sub(1, std::memory_order::acq_rel);
            reset = count <= 1;
        }

        if (reset)
        {
            m_hotData[_handle].m_nextFreeIndex = m_nextFreeIndex;
            m_nextFreeIndex = _handle;
        }

        return reset;
    }

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    HotDataStruct& SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::Get(
        KryneEngine::SimplePoolHandle _handle) const
    {
        KE_ASSERT(m_size > _handle);

        if constexpr (RefCounting)
        {
            KE_ASSERT(m_refCounts[_handle].load(std::memory_order::relaxed) > 0);
        }

        return m_hotData[_handle].m_hotData;
    }

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::ColdDataStructType&
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::GetCold(
        KryneEngine::SimplePoolHandle _handle) const
    {
        KE_ASSERT(m_size > _handle);

        if constexpr (RefCounting)
        {
            KE_ASSERT(m_refCounts[_handle].load(std::memory_order::relaxed) > 0);
        }

        return m_coldData[_handle];
    }

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::RefCountType
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::AddRef(KryneEngine::SimplePoolHandle _handle)
    {
        KE_ASSERT(m_size > _handle);
        return m_refCounts[_handle].fetch_add(1, std::memory_order::relaxed) + 1;
    }

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::RefCountType
    SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::GetRefCount(KryneEngine::SimplePoolHandle _handle)
    {
        KE_ASSERT(m_size > _handle);
        return m_refCounts[_handle].load(std::memory_order::acquire);
    }

    template <class HotDataStruct, class ColdDataStruct, bool RefCounting, class Allocator>
    void SimplePool<HotDataStruct, ColdDataStruct, RefCounting, Allocator>::Resize(size_t _toSize)
    {
        KE_ASSERT_MSG(m_size < _toSize, "Simple pool is meant to only grow");

        {
            HotDataItem* newHotData = m_allocator.allocate(_toSize * sizeof(HotDataItem));

            if (m_hotData != nullptr)
            {
                memcpy(newHotData, m_hotData,  m_size * sizeof(HotDataItem));
                m_allocator.deallocate(m_hotData, m_size * sizeof(HotDataItem));
            }

            for (size_t i = m_size; i < _toSize; i++)
            {
                newHotData[i].m_nextFreeIndex = i + 1;
            }

            m_hotData = newHotData;
        }

        if constexpr (kHasColdData)
        {
            ColdDataStruct* newColdData = m_allocator.allocate(_toSize * sizeof(ColdDataStruct));

            if (m_coldData != nullptr)
            {
                memcpy(newColdData, m_coldData, m_size * sizeof(ColdDataStruct));
                m_allocator.deallocate(m_coldData, m_size * sizeof(ColdDataStruct));
            }

            m_coldData = newColdData;
        }

        if constexpr (RefCounting)
        {
            std::atomic<s32>* newRefCountArray = m_allocator.allocate(_toSize * sizeof(*m_refCounts));

            if (m_refCounts != nullptr)
            {
                for (size_t i = 0; i < m_size; i++)
                {
                    newRefCountArray[i].store(m_refCounts[i].load(std::memory_order::relaxed), std::memory_order::release);
                }
                m_allocator.deallocate(m_refCounts, m_size * sizeof(*m_refCounts));
            }

            for (size_t i = m_size; i < _toSize; i++)
            {
                newRefCountArray[i] = 0;
            }

            m_refCounts = newRefCountArray;
        }

        m_size = _toSize;
    }
}