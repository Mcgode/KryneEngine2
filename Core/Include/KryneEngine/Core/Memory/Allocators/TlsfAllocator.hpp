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
     */
    class TlsfAllocator
    {
    public:
        TlsfAllocator(const TlsfAllocator& _other) = default;
        TlsfAllocator(TlsfAllocator&& _other) = default;
        TlsfAllocator& operator=(const TlsfAllocator& _other) = default;
        TlsfAllocator& operator=(TlsfAllocator&& _other) = default;
        ~TlsfAllocator() = default;

        void* Allocate(size_t _size, size_t _alignment = 0);
        void Free(void* _ptr);

        static TlsfAllocator Create(std::byte* _heapStart, size_t _heapSize);

    protected:
        TlsfHeap::ControlBlock* m_control = nullptr;

        explicit TlsfAllocator(TlsfHeap::ControlBlock* _control): m_control(_control) {};

    private:
        void SetupHeapPool(std::byte* _heapStart, size_t _heapSize);
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
