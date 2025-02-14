/**
 * @file
 * @author Max Godefroy
 * @date 13/02/2025.
 */

#include <EASTL/unique_ptr.h>
#include <gtest/gtest.h>
#include <KryneEngine/Core/Memory/Allocators/TlsfAllocator.hpp>
#include <KryneEngine/Core/Memory/Heaps/TlsfHeap.hpp>

#include "KryneEngine/Core/Common/BitUtils.hpp"
#include "Utils/AssertUtils.hpp"

namespace KryneEngine::Tests
{
    inline TlsfHeap::ControlBlock* GetControlBlock(TlsfAllocator& _allocator)
    {
        return *reinterpret_cast<TlsfHeap::ControlBlock**>(&_allocator);
    }

    TEST(TlsfAllocator, Creation)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        constexpr size_t heapSize = 8 * 1024;
        eastl::unique_ptr<std::byte> heap(new std::byte[heapSize]);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        TlsfAllocator allocator = TlsfAllocator::Create(heap.get(), heapSize);

        const TlsfHeap::ControlBlock* control = GetControlBlock(allocator);

        // After creation, we expect the null block to have no entry in `m_nextFreeBlock`, but we expect the initial
        // pool free block to be found in `m_previousFreeBlock`
        EXPECT_EQ(control->m_nullBlock.m_nextFreeBlock, nullptr);
        EXPECT_NE(control->m_nullBlock.m_previousFreeBlock, nullptr);

        // The fl bitmap should have exactly 1 bit set, for the free block
        EXPECT_NE(control->m_flBitmap, 0);
        EXPECT_EQ(BitUtils::GetMostSignificantBit(control->m_flBitmap), BitUtils::GetLeastSignificantBit(control->m_flBitmap));
        const u8 flIndex = BitUtils::GetMostSignificantBit(control->m_flBitmap);

        // The sl bitmaps should be zero, expect the one at fl index, which should have 1 bit set, at the sl index
        for (auto i = 0; i < eastl::size(control->m_slBitmaps); i++)
        {
            if (i != flIndex)
                EXPECT_EQ(control->m_slBitmaps[i], 0);
            else
            {
                EXPECT_NE(control->m_slBitmaps[i], 0);
                EXPECT_EQ(BitUtils::GetMostSignificantBit(control->m_slBitmaps[i]), BitUtils::GetLeastSignificantBit(control->m_slBitmaps[i]));
            }
        }
        const u8 slIndex = BitUtils::GetMostSignificantBit(control->m_slBitmaps[flIndex]);

        // Check that all the header map entries are the null block, except at the free block index
        for (auto fl = 0; fl < TlsfHeap::kFlIndexCount; fl++)
        {
            for (auto sl = 0; sl < TlsfHeap::kSlCount; sl++)
            {
                if (fl != flIndex || sl != slIndex)
                    EXPECT_EQ(control->m_headerMap[fl][sl], &control->m_nullBlock);
                else
                    EXPECT_NE(control->m_headerMap[fl][sl], &control->m_nullBlock);
            }
        }

        EXPECT_EQ(control->m_headerMap[flIndex][slIndex], control->m_nullBlock.m_previousFreeBlock);

        catcher.ExpectNoMessage();
    }

    TEST(TlsfAllocator, SingleAllocate)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        constexpr size_t heapSize = 8 * 1024;
        eastl::unique_ptr<std::byte> heap(new std::byte[heapSize]);
        TlsfAllocator allocator = TlsfAllocator::Create(heap.get(), heapSize);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        const TlsfHeap::ControlBlock* control = GetControlBlock(allocator);
        const TlsfHeap::BlockHeader* firstBlock = control->m_nullBlock.m_previousFreeBlock;

        void* p0 = allocator.Allocate(1024);

        EXPECT_NE(p0, nullptr);
        EXPECT_EQ(firstBlock, TlsfHeap::UserPtrToBlockHeader(p0));

        // Similar to creation, after a single allocation we should only have 1 block
        EXPECT_NE(control->m_flBitmap, 0);
        EXPECT_EQ(BitUtils::GetMostSignificantBit(control->m_flBitmap), BitUtils::GetLeastSignificantBit(control->m_flBitmap));
        const u8 flIndex = BitUtils::GetMostSignificantBit(control->m_flBitmap);

        for (auto i = 0; i < eastl::size(control->m_slBitmaps); i++)
        {
            if (i != flIndex)
                EXPECT_EQ(control->m_slBitmaps[i], 0);
            else
            {
                EXPECT_NE(control->m_slBitmaps[i], 0);
                EXPECT_EQ(BitUtils::GetMostSignificantBit(control->m_slBitmaps[i]), BitUtils::GetLeastSignificantBit(control->m_slBitmaps[i]));
            }
        }
        const u8 slIndex = BitUtils::GetMostSignificantBit(control->m_slBitmaps[flIndex]);

        for (auto fl = 0; fl < TlsfHeap::kFlIndexCount; fl++)
        {
            for (auto sl = 0; sl < TlsfHeap::kSlCount; sl++)
            {
                if (fl != flIndex || sl != slIndex)
                    EXPECT_EQ(control->m_headerMap[fl][sl], &control->m_nullBlock);
                else
                    EXPECT_NE(control->m_headerMap[fl][sl], &control->m_nullBlock);
            }
        }

        EXPECT_NE(control->m_headerMap[flIndex][slIndex], firstBlock);

        catcher.ExpectNoMessage();
    }

    TEST(TlsfAllocator, InvalidAllocations)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        constexpr size_t heapSize = 8 * 1024;
        eastl::unique_ptr<std::byte> heap(new std::byte[heapSize]);
        TlsfAllocator allocator = TlsfAllocator::Create(heap.get(), heapSize);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        // Size 0 outputs nullptr
        EXPECT_EQ(allocator.Allocate(0), nullptr);

        // If bigger that biggest allocatable size, outputs nullptr
        EXPECT_EQ(allocator.Allocate(1ull << 60), nullptr);

        // Even if valid size, if not enough space, cannot allocate
        EXPECT_EQ(allocator.Allocate(heapSize), nullptr);

        catcher.ExpectNoMessage();
    }

    TEST(TlsfAllocator, SingleFree)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        constexpr size_t heapSize = 8 * 1024;
        eastl::unique_ptr<std::byte> heap(new std::byte[heapSize]);
        TlsfAllocator allocator = TlsfAllocator::Create(heap.get(), heapSize);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        const TlsfHeap::ControlBlock* control = GetControlBlock(allocator);
        const TlsfHeap::BlockHeader* firstBlock = control->m_nullBlock.m_previousFreeBlock;

        const size_t firstBlockSize = firstBlock->GetSize();

        void* p = allocator.Allocate(1024);
        EXPECT_NE(p, nullptr);

        allocator.Free(p);

        // Similar to single allocation, after a single alloc & free we should only have 1 block
        EXPECT_NE(control->m_flBitmap, 0);
        EXPECT_EQ(BitUtils::GetMostSignificantBit(control->m_flBitmap), BitUtils::GetLeastSignificantBit(control->m_flBitmap));
        const u8 flIndex = BitUtils::GetMostSignificantBit(control->m_flBitmap);

        for (auto i = 0; i < eastl::size(control->m_slBitmaps); i++)
        {
            if (i != flIndex)
                EXPECT_EQ(control->m_slBitmaps[i], 0);
            else
            {
                EXPECT_NE(control->m_slBitmaps[i], 0);
                EXPECT_EQ(BitUtils::GetMostSignificantBit(control->m_slBitmaps[i]), BitUtils::GetLeastSignificantBit(control->m_slBitmaps[i]));
            }
        }
        const u8 slIndex = BitUtils::GetMostSignificantBit(control->m_slBitmaps[flIndex]);

        for (auto fl = 0; fl < TlsfHeap::kFlIndexCount; fl++)
        {
            for (auto sl = 0; sl < TlsfHeap::kSlCount; sl++)
            {
                if (fl != flIndex || sl != slIndex)
                    EXPECT_EQ(control->m_headerMap[fl][sl], &control->m_nullBlock);
                else
                    EXPECT_NE(control->m_headerMap[fl][sl], &control->m_nullBlock);
            }
        }

        EXPECT_EQ(control->m_headerMap[flIndex][slIndex], firstBlock);
        EXPECT_EQ(firstBlock->GetSize(), firstBlockSize);

        catcher.ExpectNoMessage();
    }
}