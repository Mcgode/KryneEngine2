/**
 * @file
 * @author Max Godefroy
 * @date 02/02/2024.
 */

#include "KryneEngine/Core/Memory/GenerationalPool.hpp"

#include "KryneEngine/Core/Common/Assert.hpp"

namespace KryneEngine
{
    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::GenerationalPool(const Allocator& _allocator)
        : m_allocator(_allocator)
    {
        m_availableIndices.set_allocator(_allocator);
        _Grow(kInitialSize);
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::~GenerationalPool()
    {
        m_allocator.deallocate(m_hotDataArray, m_size * sizeof(HotData));

        if constexpr (kHasColdData)
        {
            m_allocator.deallocate(m_coldDataArray, m_size);
        }
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    void GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::_Grow(u64 _toSize)
    {
        KE_ASSERT_MSG(_toSize > m_size, "Generational pools were designed with a grow-only approach in mind.");

        KE_ASSERT_MSG(_toSize <= kMaxSize,
                      "Generational pool maximum growable size is %ull. Consider changing GenPool::IndexType to a bigger type.",
                      kMaxSize);

        auto* newHotArray = static_cast<HotData*>(m_allocator.allocate(sizeof(HotData) * _toSize, alignof(HotData)));
        // Copy old array
        if (m_hotDataArray != nullptr)
        {
            memcpy(
                reinterpret_cast<void*>(newHotArray),
                reinterpret_cast<void*>(m_hotDataArray),
                m_size * sizeof(HotData));
            m_allocator.deallocate(m_hotDataArray, m_size * sizeof(HotData));
        }
        memset(reinterpret_cast<void*>(newHotArray + m_size), 0, (_toSize - m_size) * sizeof(HotData));
        m_hotDataArray = newHotArray;

        if constexpr (kHasColdData)
        {
            auto* newColdArray = static_cast<ColdDataStruct*>(m_allocator.allocate(sizeof(ColdDataStruct) * _toSize, alignof(ColdDataStruct)));
            // Copy old array
            if (m_coldDataArray != nullptr)
            {
                memcpy(
                    reinterpret_cast<void*>(newColdArray),
                    reinterpret_cast<void*>(m_coldDataArray),
                    m_size * sizeof(ColdDataStruct));
                m_allocator.deallocate(m_coldDataArray, m_size * sizeof(ColdDataStruct));
            }
            memset(reinterpret_cast<void*>(newColdArray + m_size), 0, (_toSize - m_size) * sizeof(ColdDataStruct));
            m_coldDataArray = newColdArray;
        }

        m_availableIndices.reserve(_toSize - m_size);
        for (u64 i = _toSize; i > m_size; i--)
        {
            m_availableIndices.push_back(i - 1);
        }
        m_size = _toSize;
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    inline HotDataStruct *GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::Get(GenPool::Handle _handle)
    {
        VERIFY_OR_RETURN(_handle.m_index < m_size, nullptr);

        HotData& hotData = m_hotDataArray[_handle.m_index];
        if (hotData.m_generation != _handle.m_generation)
        {
            return nullptr;
        }
        return &hotData.m_userHotData;
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    inline eastl::pair<HotDataStruct *, ColdDataStruct *> GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::GetAll(GenPool::Handle _handle)
    {
        auto* hotData = Get(_handle);
        if (hotData == nullptr)
        {
            return eastl::pair<HotDataStruct *, ColdDataStruct *>(nullptr, nullptr);
        }
        if constexpr (kHasColdData)
        {
            return eastl::pair<HotDataStruct*, ColdDataStruct*>(hotData, &m_coldDataArray[_handle.m_index]);
        }
        else
        {
            return { hotData, nullptr };
        }
    }

    template <class HotDataStruct, class ColdDataStruct, class Allocator>
    const HotDataStruct* GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::Get(GenPool::Handle _handle) const
    {
        KE_ASSERT(_handle.m_index < m_size);

        const HotData& hotData = m_hotDataArray[_handle.m_index];
        if (hotData.m_generation != _handle.m_generation)
        {
            return nullptr;
        }
        return &hotData.m_userHotData;
    }

    template <class HotDataStruct, class ColdDataStruct, class Allocator>
    eastl::pair<const HotDataStruct*, const ColdDataStruct*>GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::GetAll(GenPool::Handle _handle) const
    {
        const HotDataStruct* hotData = Get(_handle);
        if (hotData == nullptr)
        {
            return eastl::pair<const HotDataStruct *, const ColdDataStruct *>(nullptr, nullptr);
        }
        if constexpr (kHasColdData)
        {
            return eastl::pair<const HotDataStruct*, const ColdDataStruct*>(hotData, &m_coldDataArray[_handle.m_index]);
        }
        else
        {
            return eastl::pair<const HotDataStruct*, const ColdDataStruct*>{ hotData, nullptr };
        }
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    GenPool::Handle GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::Allocate()
    {
        if (m_availableIndices.empty())
        {
            VERIFY_OR_RETURN(m_size < kMaxSize, GenPool::kInvalidHandle);
            _Grow(m_size * 2);
        }

        // Pop the top of the index stack
        GenPool::IndexType index = m_availableIndices.back();
        m_availableIndices.pop_back();

        return { index, m_hotDataArray[index].m_generation };
    }

    template<class HotDataStruct, class ColdDataStruct, class Allocator>
    bool GenerationalPool<HotDataStruct, ColdDataStruct, Allocator>::Free(const GenPool::Handle &_handle,
                                                               HotDataStruct *_hotCopy,
                                                               ColdDataStruct *_coldCopy)
    {
        auto& hotData = m_hotDataArray[_handle.m_index];
        if (hotData.m_generation != _handle.m_generation)
        {
            return false;
        }

        if (_hotCopy != nullptr)
        {
            *_hotCopy = hotData.m_userHotData;
        }

        if constexpr (kHasColdData)
        {
            if (_coldCopy != nullptr)
            {
                *_coldCopy = m_coldDataArray[_handle.m_index];
            }
        }

        // Invalidate slot by increasing generation
        hotData.m_generation++;

        // Place back to the top of the index stack
        m_availableIndices.push_back(_handle.m_index);

        return true;
    }
} // KryneEngine