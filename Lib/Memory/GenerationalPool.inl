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
        delete m_nextPool;

        delete m_hotDataArray;

        if constexpr (kHasColdData)
        {
            delete m_coldDataArray;
        }
    }

    template<class HotDataStruct, class ColdDataStruct>
    void GenerationalPool<HotDataStruct, ColdDataStruct>::_Grow(u64 _toSize)
    {
        KE_ASSERT_MSG(_toSize <= m_size, "Generational pools were designed with a grow-only approach in mind.");

        KE_ASSERT_MSG(_toSize <= kMaxSize,
                      "Generational pool maximum growable size is %ull. Consider changing GenPool::IndexType to a bigger type.",
                      kMaxSize);

        u8* newHotArray = new HotData[_toSize];
        // Copy old array
        if (m_hotDataArray != nullptr)
        {
            memcpy(newHotArray, m_hotDataArray, m_size * sizeof(HotData));
            delete m_hotDataArray;
        }
        memset(newHotArray + m_size * sizeof(HotData), 0, (_toSize - m_size) * sizeof(HotData));
        m_hotDataArray = newHotArray;

        if constexpr (kHasColdData)
        {
            u8* newColdArray = new ColdDataStruct[_toSize];
            // Copy old array
            if (m_coldDataArray != nullptr)
            {
                memcpy(newColdArray, m_coldDataArray, m_size * sizeof(ColdDataStruct));
                delete m_coldDataArray;
            }
            memset(newColdArray + m_size * sizeof(ColdDataStruct), 0, (_toSize - m_size) * sizeof(ColdDataStruct));
            m_coldDataArray = newColdArray;
        }
    }

    template<class HotDataStruct, class ColdDataStruct>
    inline HotDataStruct *GenerationalPool<HotDataStruct, ColdDataStruct>::Get(const GenPool::Handle &_handle)
    {
        KE_ASSERT(_handle.m_index < m_size);

        HotData& hotData = m_hotDataArray[_handle.m_index];
        if (hotData.m_generation != _handle.m_generation)
        {
            return nullptr;
        }
        return &hotData.m_userHotData;
    }

    template<class HotDataStruct, class ColdDataStruct>
    inline eastl::pair<HotDataStruct *, ColdDataStruct *> GenerationalPool<HotDataStruct, ColdDataStruct>::GetAll(const GenPool::Handle &_handle)
    {
        auto* hotData = Get(_handle);
        if (hotData == nullptr)
        {
            return eastl::pair<HotDataStruct *, ColdDataStruct *>(nullptr, nullptr);
        }
        return eastl::pair<HotDataStruct *, ColdDataStruct *>(hotData, &m_coldDataArray[_handle.m_index]);
    }

    template<class HotDataStruct, class ColdDataStruct>
    GenPool::Handle GenerationalPool<HotDataStruct, ColdDataStruct>::Allocate()
    {
        if (m_availableIndices.empty())
        {
            _Grow(m_size * 2);
        }

        // Pop the top of the index stack
        GenPool::IndexType index = m_availableIndices.back();
        m_availableIndices.pop_back();

        return { index, m_hotDataArray[index].m_generation };
    }

    template<class HotDataStruct, class ColdDataStruct>
    void GenerationalPool<HotDataStruct, ColdDataStruct>::Free(const GenPool::Handle &_handle)
    {
        auto& hotData = m_hotDataArray[_handle.m_index];
        if (hotData.m_generation != _handle.m_generation)
        {
            return;
        }

        // Invalidate slot by increasing generation
        hotData.m_generation++;

        // Place back to the top of the index stack
        m_availableIndices.push_back(_handle.m_index);
    }
} // KryneEngine