/**
 * @file
 * @author Max Godefroy
 * @date 10/02/2025.
 */

#pragma once

#include "KryneEngine/Core/Common/Types.hpp"

namespace KryneEngine
{
    namespace TlsfHeap
    {
        struct BlockHeader;
        struct ControlBlock;
    }

    /**
     * @brief A heap allocator with linear cost allocation & free, with reduced fragmentation.
     *
     * @details
     * Based on http://www.gii.upv.es/tlsf/index.html
     *
     * The allocator needs to be provided with an initial heap to be created. The class instance itself will be stored
     * at the start of the initial heap, and will be followed by the TLSF heap control block, with the remaining of the
     * heap used as the initial heap pool.
     * This allows the allocator to be fully accounted for memory-wise.
     */
    class TlsfAllocator
    {
    public:
        TlsfAllocator(const TlsfAllocator& _other) = delete;
        TlsfAllocator(TlsfAllocator&& _other) = delete;
        TlsfAllocator& operator=(const TlsfAllocator& _other) = delete;
        TlsfAllocator& operator=(TlsfAllocator&& _other) = delete;
        ~TlsfAllocator() = delete;

        void* Allocate(size_t _size, size_t _alignment = 0);
        void Free(void* _ptr);

        static TlsfAllocator* Create(std::byte* _heapStart, size_t _heapSize);

    protected:
        explicit TlsfAllocator(size_t _allocatorSize): m_allocatorSize(_allocatorSize) {}

        void SetupHeapPool(std::byte* _heapStart, size_t _heapSize);
        [[nodiscard]] inline TlsfHeap::ControlBlock* GetControlBlock() const
        {
            return reinterpret_cast<TlsfHeap::ControlBlock*>(reinterpret_cast<uintptr_t>(this) + m_allocatorSize);
        }

    private:
        size_t m_allocatorSize;

        void InsertBlock(TlsfHeap::BlockHeader* _block);
        void RemoveBlock(TlsfHeap::BlockHeader* _block, u8 _fl, u8 _sl);
        static TlsfHeap::BlockHeader* LinkNext(TlsfHeap::BlockHeader* _block);
        static TlsfHeap::BlockHeader* NextBlock(const TlsfHeap::BlockHeader* _block);
        static bool CanSplit(const TlsfHeap::BlockHeader* _block, size_t _size);
        static TlsfHeap::BlockHeader* SplitBlock(TlsfHeap::BlockHeader* _block, size_t _size);

        static eastl::pair<u8, u8> MappingInsert(u64 _insertSize);
        static eastl::pair<u8, u8> MappingSearch(u64 _desiredSize);

        TlsfHeap::BlockHeader* SearchHeader(u64 _desiredSize, u8& _fl, u8& _sl);
        void* PrepareBlockUsed(TlsfHeap::BlockHeader* _block, size_t _size);
        void TrimFree(TlsfHeap::BlockHeader* _block, size_t _size);
        static void MarkAsFree(TlsfHeap::BlockHeader* _block);
        static void MarkAsUsed(TlsfHeap::BlockHeader* _block);

        TlsfHeap::BlockHeader* MergePreviousBlock(TlsfHeap::BlockHeader* _block);
        TlsfHeap::BlockHeader* MergeNextBlock(TlsfHeap::BlockHeader* _block);
        static TlsfHeap::BlockHeader* MergeBlocks(TlsfHeap::BlockHeader* _left, TlsfHeap::BlockHeader* _right);
    };
} // namespace KryneEngine
