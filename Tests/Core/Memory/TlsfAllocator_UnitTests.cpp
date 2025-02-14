/**
 * @file
 * @author Max Godefroy
 * @date 13/02/2025.
 */

#include <gtest/gtest.h>
#include <KryneEngine/Core/Memory/Allocators/TlsfAllocator.hpp>
#include <KryneEngine/Core/Memory/Heaps/TlsfHeap.hpp>

#include "KryneEngine/Core/Common/BitUtils.hpp"
#include "Utils/AssertUtils.hpp"

namespace KryneEngine::Tests
{
    TEST(TlsfAllocator, Creation)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        std::byte heap[8 * 1024];
        TlsfAllocator allocator = TlsfAllocator::Create(heap, sizeof(heap));

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        TlsfHeap::ControlBlock* control = *reinterpret_cast<TlsfHeap::ControlBlock**>(&allocator);

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

        catcher.ExpectNoMessage();
    }


}