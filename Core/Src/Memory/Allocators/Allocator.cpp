/**
 * @file
 * @author Max Godefroy
 * @date 15/02/2025.
 */

#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"

#include "KryneEngine/Core/Platform/StdAlloc.hpp"

namespace KryneEngine
{
    void* AllocatorInstance::allocate(size_t _size, int _flags)
    {
        if (m_allocator)
        {
            return m_allocator->Allocate(_size, 0);
        }
        else
        {
            return StdAlloc::Malloc(_size);
        }
    }

    void* AllocatorInstance::allocate(size_t _size, size_t _alignment, size_t _alignmentOffset, int _flags)
    {
        void* ptr;
        if (m_allocator)
        {
            ptr = m_allocator->Allocate(_size, _alignment);
        }
        else
        {
            ptr = StdAlloc::MemAlign(_size, _alignment);
        }
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) + _alignmentOffset);
    }

    void AllocatorInstance::deallocate(void* _ptr, size_t _size)
    {
        if (m_allocator)
        {
            m_allocator->Free(_ptr, _size);
        }
        else
        {
            StdAlloc::Free(_ptr);
        }
    }
} // namespace KryneEngine