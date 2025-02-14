/**
 * @file
 * @author Max Godefroy
 * @date 10/02/2025.
 */

#include "KryneEngine/Core/Memory/Allocators/TlsfAllocator.hpp"

#include <cstring>
#include <stdexcept>

#include "KryneEngine/Core/Common/BitUtils.hpp"
#include "KryneEngine/Core/Common/Utils/Alignment.hpp"
#include "KryneEngine/Core/Memory/Heaps/TlsfHeap.hpp"

#if !defined(TLSF_HEAP_ASSERT)
#   define TLSF_HEAP_ASSERT 1
#endif

#if TLSF_HEAP_ASSERT
#   define TLSF_ASSERT(cond) KE_ASSERT_FATAL(cond)
#   define TLSF_ASSERT_MSG(cond, ...) KE_ASSERT_FATAL_MSG(cond, __VA_ARGS__)
#else
#   define TLSF_ASSERT(cond)
#   define TLSF_ASSERT_MSG(cond, ...)
#endif

namespace KryneEngine
{
    TlsfAllocator TlsfAllocator::Create(std::byte* _heapStart, size_t _heapSize)
    {
        TLSF_ASSERT_MSG(_heapSize > sizeof(TlsfHeap::ControlBlock), "Heap size must be greater than the size of the control block");

        auto* control = reinterpret_cast<TlsfHeap::ControlBlock*>(_heapStart);
        memset(control, 0, offsetof(TlsfHeap::ControlBlock, m_headerMap));
        for (auto& headerList : control->m_headerMap)
        {
            for (auto& header : headerList)
            {
                header = &control->m_nullBlock;
            }
        }
        memset(&control->m_userData, 0, sizeof(control->m_userData));

        TlsfAllocator allocator(control);
        allocator.SetupHeapPool(_heapStart + sizeof(TlsfHeap::ControlBlock), _heapSize - sizeof(TlsfHeap::ControlBlock));
        return allocator;
    }

    void* TlsfAllocator::Allocate(size_t _size, size_t _alignment)
    {
        TlsfHeap::BlockHeader* header = SearchHeader(_size);
        return PrepareBlockUsed(header, _size);
    }

    void TlsfAllocator::Free(void* _ptr)
    {

    }

    void TlsfAllocator::SetupHeapPool(std::byte* _heapStart, size_t _heapSize)
    {
        using namespace TlsfHeap;

        const size_t heapPoolBytes = Alignment::AlignDownPot(_heapSize - kHeapPoolOverhead, kAlignmentPot);

        TLSF_ASSERT_MSG(
            Alignment::IsAligned(reinterpret_cast<uintptr_t>(_heapStart), kAlignment),
            "Heap start must be aligned to %lld bytes", kAlignment);

        TLSF_ASSERT_MSG(
            heapPoolBytes >= kMinBlockSize && heapPoolBytes <= kMaxBlockSize,
            "Heap pool size must be contained between 0x%x and 0x%x00 bytes",
            kHeapPoolOverhead + kMinBlockSize,
            (kHeapPoolOverhead + kMaxBlockSize) / 256
            );

        auto* block = reinterpret_cast<BlockHeader*>(_heapStart - kBlockHeaderMemoryAddressLeftOffset);
        block->SetSize(heapPoolBytes);
        block->SetFree();
        block->SetPrevUsed();
        InsertBlock(block);

        BlockHeader* next = LinkNext(block);
        next->SetSize(0);
        next->SetUsed();
        next->SetPrevFree();
    }

    void TlsfAllocator::InsertBlock(TlsfHeap::BlockHeader* _block)
    {
        TLSF_ASSERT_MSG(_block->IsFree(), "Block must be free");
        const auto [fl, sl] = MappingInsert(_block->GetSize());

        TlsfHeap::BlockHeader* current = m_control->m_headerMap[fl][sl];
        TLSF_ASSERT_MSG(current != nullptr, "Freelist cannot have a null entry");
        TLSF_ASSERT_MSG(_block != nullptr, "Cannot insert a null entry in the freelist");
        _block->m_nextFreeBlock = current;
        _block->m_previousFreeBlock = &m_control->m_nullBlock;
        current->m_previousFreeBlock = _block;

        TLSF_ASSERT_MSG(
            Alignment::IsAligned(reinterpret_cast<uintptr_t>(TlsfHeap::BlockHeaderToUserPtr(_block)), TlsfHeap::kAlignment),
            "Block not aligned properly");

        m_control->m_headerMap[fl][sl] = _block;
        m_control->m_flBitmap |= (1 << fl);
        m_control->m_slBitmaps[fl] |= (1 << sl);
    }

    TlsfHeap::BlockHeader* TlsfAllocator::LinkNext(TlsfHeap::BlockHeader* _block)
    {
        TlsfHeap::BlockHeader* next = NextBlock(_block);
        next->m_previousPhysicalBlock = _block;
        return next;
    }

    TlsfHeap::BlockHeader* TlsfAllocator::NextBlock(const TlsfHeap::BlockHeader* _block)
    {
        auto* next = reinterpret_cast<TlsfHeap::BlockHeader*>(
            reinterpret_cast<uintptr_t>(TlsfHeap::BlockHeaderToUserPtr(_block)) +
            _block->GetSize() - TlsfHeap::kBlockHeaderOverhead);
        TLSF_ASSERT(!_block->IsLast());
        return next;
    }

    bool TlsfAllocator::CanSplit(const TlsfHeap::BlockHeader* _block, size_t _size)
    {
        return _block->GetSize() > _size + sizeof(TlsfHeap::BlockHeader);
    }

    TlsfHeap::BlockHeader* TlsfAllocator::SplitBlock(TlsfHeap::BlockHeader* _block, size_t _size)
    {
        auto* remaining = reinterpret_cast<TlsfHeap::BlockHeader*>(
            reinterpret_cast<uintptr_t>(TlsfHeap::BlockHeaderToUserPtr(_block))
            + _size - TlsfHeap::kBlockHeaderOverhead);
        const size_t remainingSize = _block->GetSize() - (_size + TlsfHeap::kBlockHeaderOverhead);

        TLSF_ASSERT_MSG(
            Alignment::IsAligned(reinterpret_cast<uintptr_t>(TlsfHeap::BlockHeaderToUserPtr(remaining)), TlsfHeap::kAlignment),
            "Remaining block not aligned properly");

        TLSF_ASSERT(_block->GetSize() == _size + remainingSize + TlsfHeap::kBlockHeaderOverhead);
        remaining->SetSize(remainingSize);
        TLSF_ASSERT_MSG(remaining->GetSize() >= TlsfHeap::kMinBlockSize, "Remaining block must be at least %lld bytes", TlsfHeap::kSmallBlockSize);

        _block->SetSize(_size);
        MarkAsFree(remaining);

        return remaining;
    }

    eastl::pair<u8, u8> TlsfAllocator::MappingInsert(u64 _insertSize)
    {
        if (_insertSize < TlsfHeap::kSmallBlockSize)
        {
            // Store small blocks in first list.
            return { 0, _insertSize / (TlsfHeap::kSmallBlockSize / TlsfHeap::kSlCount) };
        }
        const u8 fl = BitUtils::GetMostSignificantBit( _insertSize);
        const u8 sl = (_insertSize >> (fl - TlsfHeap::kSlCountPot)) - (1 << TlsfHeap::kSlCountPot);
        return { fl - TlsfHeap::kFlShift + 1, sl };
    }

    eastl::pair<u8, u8> TlsfAllocator::MappingSearch(u64 _desiredSize)
    {
        // Round up instead of rounding down
        _desiredSize += (1 << (BitUtils::GetMostSignificantBit(_desiredSize) - TlsfHeap::kSlCountPot)) - 1;
        return MappingInsert(_desiredSize);
    }

    TlsfHeap::BlockHeader* TlsfAllocator::SearchHeader(u64 _desiredSize)
    {
        const auto [fl, sl] = MappingSearch(_desiredSize);
        u64 bitmap = m_control->m_slBitmaps[fl] & (~0 << sl);

        u8 selectedFl = 255;
        u8 selectedSl;
        if (bitmap != 0)
        {
            selectedFl = fl;
            selectedSl = BitUtils::GetLeastSignificantBit(bitmap);
        }
        else
        {
            bitmap = m_control->m_flBitmap & (~0 << (fl + 1));
            if (bitmap != 0)
            {
                selectedFl = BitUtils::GetLeastSignificantBit(bitmap);
                selectedSl = BitUtils::GetLeastSignificantBit(m_control->m_slBitmaps[selectedFl]);
            }
        }

        if (selectedFl != 255)
            return m_control->m_headerMap[selectedFl][selectedSl];
        return nullptr;
    }

    void* TlsfAllocator::PrepareBlockUsed(TlsfHeap::BlockHeader* _block, size_t _size)
    {
        if (_block == nullptr)
            return nullptr;

        TLSF_ASSERT_MSG(_size > 0, "Size must be non-zero");
        TrimFree(_block, _size);
        MarkAsUsed(_block);
        return TlsfHeap::BlockHeaderToUserPtr(_block);
    }

    void TlsfAllocator::TrimFree(TlsfHeap::BlockHeader* _block, size_t _size)
    {
        TLSF_ASSERT_MSG(_block->IsFree(), "Block must be free");

        if (CanSplit(_block, _size))
        {
            TlsfHeap::BlockHeader* remaining = SplitBlock(_block, _size);
            LinkNext(_block);
            remaining->SetPrevFree();
            InsertBlock(remaining);
        }
    }

    void TlsfAllocator::MarkAsFree(TlsfHeap::BlockHeader* _block)
    {
        TlsfHeap::BlockHeader* next = NextBlock(_block);
        next->SetPrevFree();
        _block->SetFree();
    }

    void TlsfAllocator::MarkAsUsed(TlsfHeap::BlockHeader* _block)
    {
        TlsfHeap::BlockHeader* next = NextBlock(_block);
        next->SetPrevUsed();
        _block->SetUsed();
    }
} // namespace KryneEngine