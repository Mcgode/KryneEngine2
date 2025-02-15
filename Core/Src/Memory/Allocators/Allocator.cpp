/**
 * @file
 * @author Max Godefroy
 * @date 15/02/2025.
 */

#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"

#include <EASTL/allocator.h>

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
            return ::new((char*)nullptr, _flags, 0, __FILE__, __LINE__) std::byte[_size];
        }
    }

    void* AllocatorInstance::allocate(size_t _size, size_t _alignment, size_t _alignmentOffset, int _flags)
    {
        if (m_allocator)
        {
            return reinterpret_cast<void*>(
                reinterpret_cast<uintptr_t>(m_allocator->Allocate(_size, _alignment)) + _alignmentOffset);
        }
        else
        {
            return ::new(_alignment, _alignmentOffset, (char*)nullptr, _flags, 0, __FILE__, __LINE__) std::byte[_size];
        }
    }

    void AllocatorInstance::deallocate(void* _ptr, size_t _size)
    {
        if (m_allocator)
        {
            m_allocator->Free(_ptr, _size);
        }
        else
        {
            ::operator delete[](_ptr);
        }
    }
} // namespace KryneEngine