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

            static Handle FromU32(u32 _rawHandle)
            {
                Handle handle {};
                *reinterpret_cast<u32*>(&handle) = _rawHandle;
                return handle;
            }
        };

        static constexpr Handle kInvalidHandle = {
                static_cast<IndexType>(0),
                static_cast<GenerationType>(~0)
        };
    }

#define KE_GENPOOL_DECLARE_HANDLE(HandleName) struct HandleName                                 \
    {                                                                                           \
        GenPool::Handle m_handle = GenPool::kInvalidHandle;                                     \
                                                                                                \
        HandleName& operator=(GenPool::Handle _other) { m_handle = _other; return *this; }      \
        bool operator==(GenPool::Handle _other) const { return m_handle == _other; }            \
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

        HotDataStruct* Get(GenPool::Handle _handle);
        eastl::pair<HotDataStruct*, ColdDataStruct*> GetAll(GenPool::Handle _handle);
        inline ColdDataStruct* GetCold(GenPool::Handle _handle)
        {
            return GetAll(_handle).second;
        }

        const HotDataStruct* Get(GenPool::Handle _handle) const;
        eastl::pair<const HotDataStruct*, const ColdDataStruct*> GetAll(GenPool::Handle _handle) const;
        inline const ColdDataStruct* GetCold(const GenPool::Handle& _handle) const
        {
            return GetAll(_handle).second;
        }

        GenPool::Handle Allocate();
        bool Free(const GenPool::Handle &_handle,
                  HotDataStruct *_hotCopy = nullptr,
                  ColdDataStruct *_coldCopy = nullptr);

        [[nodiscard]] size_t GetSize() const { return m_size; }
    };

} // KryneEngine