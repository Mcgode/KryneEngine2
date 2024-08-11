/**
 * @file
 * @author Max Godefroy
 * @date 02/02/2024.
 */

#pragma once

#include <Common/Types.hpp>
#include <EASTL/span.h>
#include <Common/Arrays.hpp>
#include <Threads/LightweightMutex.hpp>

namespace KryneEngine
{
    namespace GenPool
    {
        using IndexType = u16;
        using GenerationType = u16;

        struct Handle
        {
            IndexType m_index;
            GenerationType m_generation;

            inline bool operator==(const Handle &rhs) const
            {
                return static_cast<u32>(*this) == static_cast<u32>(rhs);
            }

            inline explicit operator u32() const
            {
                static_assert(sizeof(Handle) == sizeof(u32));
                return *reinterpret_cast<const u32*>(this);
            }
        };

        static constexpr Handle kInvalidHandle = {
                static_cast<IndexType>(0),
                static_cast<GenerationType>(~0)
        };
    }

    template <class HotDataStruct, class ColdDataStruct = void>
    class GenerationalPool
    {
        struct HotData
        {
            HotDataStruct m_userHotData;
            GenPool::GenerationType m_generation;
        };

        static constexpr u64 kInitialSize = 32;
        static constexpr u64 kMaxSize = 1 << 16;
        static constexpr bool kHasColdData = !eastl::is_same<void, ColdDataStruct>::value;

        HotData* m_hotDataArray = nullptr;
        ColdDataStruct* m_coldDataArray = nullptr;

        u64 m_size = 0;

        eastl::vector<GenPool::IndexType> m_availableIndices;

        void _Grow(u64 _toSize);

    public:
        GenerationalPool();

        ~GenerationalPool();

        HotDataStruct* Get(const GenPool::Handle& _handle);
        eastl::pair<HotDataStruct*, ColdDataStruct*> GetAll(const GenPool::Handle& _handle);
        inline ColdDataStruct* GetCold(const GenPool::Handle& _handle)
        {
            return GetAll(_handle).second;
        }

        const HotDataStruct* Get(const GenPool::Handle& _handle) const;
        eastl::pair<const HotDataStruct*, const ColdDataStruct*> GetAll(GenPool::Handle _handle) const;
        inline const ColdDataStruct* GetCold(const GenPool::Handle& _handle) const
        {
            return GetAll(_handle).second;
        }

        GenPool::Handle Allocate();
        bool Free(const GenPool::Handle &_handle,
                  HotDataStruct *_hotCopy = nullptr,
                  ColdDataStruct *_coldCopy = nullptr);
    };

} // KryneEngine