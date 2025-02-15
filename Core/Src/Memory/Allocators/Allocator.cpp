/**
 * @file
 * @author Max Godefroy
 * @date 15/02/2025.
 */

#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"

#include <EASTL/allocator.h>

namespace KryneEngine
{
    void* AllocatorInstance::allocate(const size_t _size, const u32 _flags)
    {
        if (m_allocator)
        {
            return m_allocator->Allocate(_size, 0);
        }
        else
        {
            return ::new((char*)nullptr, static_cast<s32>(_flags), 0, __FILE__, __LINE__) std::byte[_size];
        }
    }

    void* AllocatorInstance::allocate(size_t _size, size_t _alignment, size_t _alignmentOffset, u32 _flags)
    {
        if (m_allocator)
        {
            return reinterpret_cast<void*>(
                reinterpret_cast<uintptr_t>(m_allocator->Allocate(_size, _alignment)) + _alignmentOffset);
        }
        else
        {
            return ::new(_alignment, _alignmentOffset, (char*)nullptr, static_cast<s32>(_flags), 0, __FILE__, __LINE__) std::byte[_size];
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