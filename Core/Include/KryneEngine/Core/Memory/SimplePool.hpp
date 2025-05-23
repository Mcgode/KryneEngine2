/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#pragma once

#include <atomic>

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"

namespace KryneEngine
{
    using SimplePoolHandle = size_t;

    template <class HotDataStruct, class ColdDataStruct = void, bool RefCounting = false, class Allocator = AllocatorInstance>
    class SimplePool
    {
    public:
        SimplePool();
        explicit SimplePool(const Allocator& _allocator);
        SimplePool(const Allocator& _allocator, size_t _initialSize);

        ~SimplePool();

        SimplePool(const SimplePool&) = delete;
        SimplePool(SimplePool&&) noexcept = delete;
        SimplePool& operator=(const SimplePool&) = delete;
        SimplePool& operator=(SimplePool&&) noexcept = delete;

        static constexpr bool kHasColdData = !eastl::is_void_v<ColdDataStruct>;

        SimplePoolHandle Allocate();

        template<class... Args>
        SimplePoolHandle AllocateAndInit(Args... _args);

        bool Free(SimplePoolHandle  _handle, HotDataStruct* _hotCopy = nullptr, ColdDataStruct* _coldDataCopy = nullptr);

        HotDataStruct& Get(SimplePoolHandle _handle) const;

        eastl::add_lvalue_reference_t<ColdDataStruct> GetCold(SimplePoolHandle _handle) const requires kHasColdData;

        s32 AddRef(SimplePoolHandle _handle) requires RefCounting;
        [[nodiscard]] s32 GetRefCount(SimplePoolHandle _handle) const requires RefCounting;

        [[nodiscard]] const Allocator& GetAllocator() const { return m_allocator; }
        void SetAllocator(const Allocator& _allocator) { m_allocator = _allocator; }

    protected:
        static constexpr size_t kDefaultPoolSize = 32;

        void Resize(size_t _toSize);

    private:
        union HotDataItem;

        Allocator m_allocator;
        HotDataItem* m_hotData = nullptr;
        ColdDataStruct* m_coldData = nullptr;
        std::atomic<s32>* m_refCounts = nullptr;
        size_t m_size = 0;
        SimplePoolHandle m_nextFreeIndex = 0;
    };

}