/**
 * @file
 * @author Max Godefroy
 * @date 25/02/2025.
 */

#include "KryneEngine/Core/Memory/Allocators/DefaultHeapHeapAllocationTracker.hpp"

#if KE_TRACK_DEFAULT_HEAP_ALLOCATIONS

#include "KryneEngine/Core/Platform/StdAlloc.hpp"

namespace KryneEngine
{
    DefaultHeapHeapAllocationTracker DefaultHeapHeapAllocationTracker::s_instance {};

    void DefaultHeapHeapAllocationTracker::RegisterAllocation(void* _ptr, size_t _size, size_t _alignment)
    {
        const auto lock = m_mutex.AutoLock();
        const size_t index = m_allocations.size();
        m_allocations.emplace_back(_size, _alignment, false);
        m_allocationMap.emplace(_ptr, index);
    }

    void DefaultHeapHeapAllocationTracker::RegisterDeallocation(void* _ptr)
    {
        const auto lock = m_mutex.AutoLock();
        const auto it = m_allocationMap.find(_ptr);
        if (it != m_allocationMap.end())
        {
            m_allocations[it->second].m_freed = true;
            m_allocationMap.erase(it);
        }
    }

    void* DefaultHeapHeapAllocationTracker::CustomAllocator::allocate(size_t _size, int)
    {
        return StdAlloc::Malloc(_size);
    }

    void* DefaultHeapHeapAllocationTracker::CustomAllocator::allocate(
        size_t _size,
        size_t _alignment,
        size_t _alignmentOffset,
        int)
    {
        return reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(StdAlloc::MemAlign(_size, _alignment)) + _alignmentOffset);
    }

    void DefaultHeapHeapAllocationTracker::CustomAllocator::deallocate(
        void* _ptr,
        size_t _size)
    {
        StdAlloc::Free(_ptr);
    }
}

#endif