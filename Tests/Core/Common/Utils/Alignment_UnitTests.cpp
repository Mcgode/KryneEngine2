/**
 * @file
 * @author Max Godefroy
 * @date 18/09/2024.
 */

#include <gtest/gtest.h>
#include <KryneEngine/Core/Common/Utils/Alignment.hpp>

namespace KryneEngine::Tests
{
    TEST(Alignment, IsAligned)
    {
        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_TRUE(Alignment::IsAligned(4, 4));

        EXPECT_FALSE(Alignment::IsAligned(5, 4));

        EXPECT_TRUE(Alignment::IsAligned(128, 4));

        EXPECT_FALSE(Alignment::IsAligned(128, 5));

        EXPECT_TRUE(Alignment::IsAligned(123, 3));

        // Always aligned to 1
        EXPECT_TRUE(Alignment::IsAligned(123456789, 1));

        // Cannot be aligned to zero
        EXPECT_FALSE(Alignment::IsAligned(123456789, 0));

        // Alignment also works for negative numbers, though we expect to work with unsigned numbers
        EXPECT_TRUE(Alignment::IsAligned(128, -4));
    }

    TEST(Alignment, AlignUp)
    {
        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_EQ(Alignment::AlignUp(4, 4), 4);

        EXPECT_EQ(Alignment::AlignUp(5, 4), 8);

        EXPECT_EQ(Alignment::AlignUp(128, 4), 128);

        EXPECT_EQ(Alignment::AlignUp(128, 5), 130);

        EXPECT_EQ(Alignment::AlignUp(123, 3), 123);

        EXPECT_EQ(Alignment::AlignUp(123456789, 1), 123456789);

        EXPECT_EQ(Alignment::AlignUp(123456789, 0), 0);

        // AlignUp is not designed to support negative numbers, this is expected
        EXPECT_NE(Alignment::AlignUp(128, -4), 128);
    }

    TEST(Alignment, AlignUpPot)
    {
        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_EQ(Alignment::AlignUpPot(4, 2), 4);

        EXPECT_EQ(Alignment::AlignUpPot(5, 2), 8);

        EXPECT_EQ(Alignment::AlignUpPot(128, 5), 128);

        EXPECT_EQ(Alignment::AlignUpPot(128, 1), 128);

        EXPECT_EQ(Alignment::AlignUpPot(123, 3), 128);

        EXPECT_EQ(Alignment::AlignUpPot(123456789, 0), 123456789);

        EXPECT_EQ(Alignment::AlignUpPot(128, 10), 1024);
    }

    TEST(Alignment, NextPowerOfTwo)
    {
        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_EQ(Alignment::NextPowerOfTwo(4), 4);

        EXPECT_EQ(Alignment::NextPowerOfTwo(1), 1);

        EXPECT_EQ(Alignment::NextPowerOfTwo(3), 4);

        EXPECT_EQ(Alignment::NextPowerOfTwo(129), 256);

        EXPECT_EQ(Alignment::NextPowerOfTwo(0), 0);
    }
}