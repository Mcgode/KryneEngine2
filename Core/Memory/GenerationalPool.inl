/**
 * @file
 * @author Max Godefroy
 * @date 02/02/2024.
 */

#include <Common/Assert.hpp>

#include "GenerationalPool.hpp"

namespace KryneEngine
{
    template<class HotDataStruct, class ColdDataStruct>
    GenerationalPool<HotDataStruct, ColdDataStruct>::GenerationalPool()
    {
        _Grow(kInitialSize);
    }

    template<class HotDataStruct, class ColdDataStruct>
    GenerationalPool<HotDataStruct, ColdDataStruct>::~GenerationalPool()
    {
        delete[] m_hotDataArray;

        if constexpr (kHasColdData)
        {
            delete[] m_coldDataArray;
        }
    }

    template<class HotDataStruct, class ColdDataStruct>
    void GenerationalPool<HotDataStruct, ColdDataStruct>::_Grow(u64 _toSize)
    {
        KE_ASSERT_MSG(_toSize > m_size, "Generational pools were designed with a grow-only approach in mind.");

        KE_ASSERT_MSG(_toSize <= kMaxSize,
                      "Generational pool maximum growable size is %ull. Consider changing GenPool::IndexType to a bigger type.",
                      kMaxSize);

        auto* newHotArray = new HotData[_toSize];
        // Copy old array
        if (m_hotDataArray != nullptr)
        {
            memcpy(newHotArray, m_hotDataArray, m_size * sizeof(HotData));
            delete[] m_hotDataArray;
        }
        memset(newHotArray + m_size, 0, (_toSize - m_size) * sizeof(HotData));
        m_hotDataArray = newHotArray;

        if constexpr (kHasColdData)
        {
            auto* newColdArray = new ColdDataStruct[_toSize];
            // Copy old array
            if (m_coldDataArray != nullptr)
            {
                memcpy(newColdArray, m_coldDataArray, m_size * sizeof(ColdDataStruct));
                delete[] m_coldDataArray;
            }
            memset(newColdArray + m_size, 0, (_toSize - m_size) * sizeof(ColdDataStruct));
            m_coldDataArray = newColdArray;
        }

        for (u64 i = _toSize; i > m_size; i--)
        {
            m_availableIndices.push_back(i - 1);
        }
        m_size = _toSize;
    }

    template<class HotDataStruct, class ColdDataStruct>
    inline HotDataStruct *GenerationalPool<HotDataStruct, ColdDataStruct>::Get(GenPool::Handle _handle)
    {
        VERIFY_OR_RETURN(_handle.m_index < m_size, nullptr);

        HotData& hotData = m_hotDataArray[_handle.m_index];
        if (hotData.m_generation != _handle.m_generation)
        {
            return nullptr;
        }
        return &hotData.m_userHotData;
    }

    template<class HotDataStruct, class ColdDataStruct>
    inline eastl::pair<HotDataStruct *, ColdDataStruct *> GenerationalPool<HotDataStruct, ColdDataStruct>::GetAll(GenPool::Handle _handle)
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

    template <class HotDataStruct, class ColdDataStruct>
    const HotDataStruct* GenerationalPool<HotDataStruct, ColdDataStruct>::Get(GenPool::Handle _handle) const
    {
        KE_ASSERT(_handle.m_index < m_size);

        const HotData& hotData = m_hotDataArray[_handle.m_index];
        if (hotData.m_generation != _handle.m_generation)
        {
            return nullptr;
        }
        return &hotData.m_userHotData;
    }

    template <class HotDataStruct, class ColdDataStruct>
    eastl::pair<const HotDataStruct*, const ColdDataStruct*>GenerationalPool<HotDataStruct, ColdDataStruct>::GetAll(GenPool::Handle _handle) const
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

    template<class HotDataStruct, class ColdDataStruct>
    GenPool::Handle GenerationalPool<HotDataStruct, ColdDataStruct>::Allocate()
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

    template<class HotDataStruct, class ColdDataStruct>
    bool GenerationalPool<HotDataStruct, ColdDataStruct>::Free(const GenPool::Handle &_handle,
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