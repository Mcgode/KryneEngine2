/**
 * @file
 * @author Max Godefroy
 * @date 15/02/2025.
 */

#pragma once

#include <cstddef>

namespace KryneEngine
{
    class IAllocator
    {
    public:
        virtual ~IAllocator() = default;

        virtual void* Allocate(size_t _size, size_t _alignment) = 0;
        virtual void Free(void* _ptr, size_t _alignment) = 0;
    };

    struct AllocatorInstance final
    {
    public:
        AllocatorInstance() = default;
        AllocatorInstance(const AllocatorInstance&) = default;
        AllocatorInstance(AllocatorInstance&&) = default;
        AllocatorInstance& operator=(const AllocatorInstance&) = default;
        AllocatorInstance& operator=(AllocatorInstance&&) = default;
        ~AllocatorInstance() = default;

        AllocatorInstance(const char*) {};
        AllocatorInstance(AllocatorInstance& _other, const char*): m_allocator(_other.m_allocator) {};
        AllocatorInstance(IAllocator* _allocator): m_allocator(_allocator) {}

        void* allocate(size_t _size, int _flags = 0);
        void* allocate(size_t _size, size_t _alignment, size_t _alignmentOffset = 0, int _flags = 0);
        void deallocate(void* _ptr, size_t _size);

        void set_name(const char*) {}

        void SetAllocator(IAllocator* _allocator) { m_allocator = _allocator; }
        [[nodiscard]] IAllocator* GetAllocator() const { return m_allocator; }

        bool operator ==(const AllocatorInstance& _other) const { return m_allocator == _other.m_allocator; }

    private:
        IAllocator* m_allocator = nullptr;
    };
} // namespace KryneEngine
