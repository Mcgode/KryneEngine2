/**
 * @file
 * @author Max Godefroy
 * @date 08/12/2025.
 */

#pragma once

#include "StableVector.hpp"

namespace KryneEngine
{
    template <class T, size_t BlockSize, class Allocator>
    T& StableVector<T, BlockSize, Allocator>::PushBack(const T& _value)
    {
        T& entry = NextEntry();
        new (&entry) T(_value);
        return entry;
    }

    template <class T, size_t BlockSize, class Allocator>
    T& StableVector<T, BlockSize, Allocator>::PushBack(T&& _value)
    {
        T& entry = NextEntry();
        new (&entry) T(std::move(_value));
        return entry;
    }

    template <class T, size_t BlockSize, class Allocator>
    template <class... Args>
    T& StableVector<T, BlockSize, Allocator>::EmplaceBack(Args... _args)
    {
        T& entry = NextEntry();
        new (&entry) T(_args...);
        return entry;
    }

    template <class T, size_t BlockSize, class Allocator>
    void StableVector<T, BlockSize, Allocator>::Clear()
    {
        if (m_firstBlock == nullptr)
            return;

        Block* block = m_firstBlock;
        while (block != nullptr)
        {
            Block* nextBlock = block->m_next;
            m_allocator.deallocate(block, BlockSize);
            block = nextBlock;
        }
    }

    template <class T, size_t BlockSize, class Allocator>
    StableVector<T, BlockSize, Allocator>::~StableVector()
    {
        Clear();
    }

    template <class T, size_t BlockSize, class Allocator>
    T& StableVector<T, BlockSize, Allocator>::NextEntry()
    {
        const size_t localIndex = m_size % BlockSize;

        if (m_firstBlock == nullptr)
        {
            m_firstBlock = m_allocator.allocate(BlockSize);
            m_firstBlock->m_next = nullptr;
            m_lastBlock = m_firstBlock;
        }
        else if (localIndex == 0)
        {
            Block* newBlock = m_allocator.allocate(BlockSize);
            newBlock->m_next = nullptr;
            m_lastBlock->m_next = newBlock;
            m_lastBlock = newBlock;
        }

        m_size++;

        return m_lastBlock[localIndex];
    }
} // namespace KryneEngine