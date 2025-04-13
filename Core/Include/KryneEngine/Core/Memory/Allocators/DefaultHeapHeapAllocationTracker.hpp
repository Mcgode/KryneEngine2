/**
 * @file
 * @author Max Godefroy
 * @date 25/02/2025.
 */

#pragma once

#if !defined(KE_TRACK_DEFAULT_HEAP_ALLOCATIONS)
#   define KE_TRACK_DEFAULT_HEAP_ALLOCATIONS 0
#endif

#if KE_TRACK_DEFAULT_HEAP_ALLOCATIONS

#include <EASTL/hash_map.h>

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Threads/LightweightMutex.hpp"

namespace KryneEngine
{
    class DefaultHeapHeapAllocationTracker
    {
    public:
        static DefaultHeapHeapAllocationTracker& GetInstance() { return s_instance; }

        void RegisterAllocation(void* _ptr, size_t _size, size_t _alignment);
        void RegisterDeallocation(void* _ptr);

    private:
        DefaultHeapHeapAllocationTracker() = default;

        class CustomAllocator
        {
        public:
            CustomAllocator() = default;
            explicit CustomAllocator(const char* _name) {}

            void* allocate(size_t _size, int _flags = 0);
            void* allocate(size_t _size, size_t _alignment, size_t _alignmentOffset = 0, int _flags = 0);
            void deallocate(void* _ptr, size_t _size = 0);
        };

        struct AllocationInfo
        {
            size_t m_size;
            size_t m_alignment;
            bool m_freed;
        };

        using Map = eastl::hash_map<void*, size_t, eastl::hash<void*>, eastl::equal_to<void*>, CustomAllocator>;
        Map m_allocationMap;

        using Vector = eastl::vector<AllocationInfo, CustomAllocator>;
        Vector m_allocations;
        LightweightMutex m_mutex;

        static DefaultHeapHeapAllocationTracker s_instance;

    public:
        const Vector& GetAllocations() { return m_allocations; }
    };
}

#endif