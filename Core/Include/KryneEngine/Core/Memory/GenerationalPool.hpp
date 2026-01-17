/**
 * @file
 * @author Max Godefroy
 * @date 02/02/2024.
 */

#pragma once

#include "KryneEngine/Core/Memory/DynamicArray.hpp"

namespace KryneEngine
{
    namespace GenPool
    {
        static constexpr size_t kIndexBits = 20;

        struct Handle
        {
            u32 m_index: kIndexBits;
            u32 m_generation: 32 - kIndexBits;

            bool operator==(const Handle &rhs) const
            {
                return static_cast<u32>(*this) == static_cast<u32>(rhs);
            }

            explicit operator u32() const
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
                0u,
                0u
        };

        static constexpr Handle kUndefinedHandle = {
                ~0u,
                ~0u
        };
    }

#define KE_GENPOOL_DECLARE_HANDLE(HandleName) struct HandleName                                 \
    {                                                                                           \
        GenPool::Handle m_handle = GenPool::kInvalidHandle;                                     \
                                                                                                \
        HandleName& operator=(GenPool::Handle _other) { m_handle = _other; return *this; }      \
        bool operator==(GenPool::Handle _other) const { return m_handle == _other; }            \
        bool operator==(HandleName _other) const { return m_handle == _other.m_handle; }        \
    }

    template <class HotDataStruct, class ColdDataStruct = void, class Allocator = AllocatorInstance>
    class GenerationalPool
    {
        struct HotData
        {
            HotDataStruct m_userHotData;
            u32 m_generation;
        };

        static constexpr u64 kInitialSize = 32;
        static constexpr u64 kMaxSize = 1 << 20;
        static constexpr bool kHasColdData = !eastl::is_same<void, ColdDataStruct>::value;

        Allocator m_allocator {};

        HotData* m_hotDataArray = nullptr;
        ColdDataStruct* m_coldDataArray = nullptr;

        u64 m_size = 0;

        eastl::vector<u32, Allocator> m_availableIndices;

        void _Grow(u64 _toSize);

    public:
        explicit GenerationalPool(const Allocator &_allocator = Allocator());

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

        [[nodiscard]] const Allocator& GetAllocator() const { return m_allocator; }
        void SetAllocator(const Allocator &_allocator) { m_allocator = _allocator; }
    };

} // KryneEngine