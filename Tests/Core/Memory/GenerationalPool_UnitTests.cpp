/**
 * @file
 * @author Max Godefroy
 * @date 19/09/2024.
 */

#include <gtest/gtest.h>

#include <Memory/GenerationalPool.inl>
#include "Utils/AssertUtils.hpp"

namespace KryneEngine::Tests
{
    TEST(GenerationalPool, Access)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        GenerationalPool<u32> hotPool;
        GenerationalPool<u32, u32> hotAndColdPool;

        u32 expectedCaughtCount = 0;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_NE(hotPool.GetSize(), 0);
        EXPECT_NE(hotAndColdPool.GetSize(), 0);

        EXPECT_EQ(hotPool.GetSize(), hotAndColdPool.GetSize());

        EXPECT_EQ(catcher.GetCaughtMessages().size(), expectedCaughtCount);

        // Can forcibly access just initialized gen pool data with hardcoded generation set to 0.
        // In user code, we expect the user not to access it this way.
        const GenPool::Handle startHandle { 0, 0 };

        EXPECT_NE(hotPool.Get(startHandle), nullptr);
        EXPECT_NE(hotAndColdPool.Get(startHandle), nullptr);

        EXPECT_EQ(hotPool.GetAll(startHandle).first, hotPool.Get(startHandle));
        EXPECT_EQ(hotPool.GetAll(startHandle).second, nullptr);

        EXPECT_NE(hotAndColdPool.GetAll(startHandle).second, nullptr);
        EXPECT_EQ(hotAndColdPool.GetAll(startHandle).second, hotAndColdPool.GetCold(startHandle));

        // Should have received no assert
        EXPECT_EQ(catcher.GetCaughtMessages().size(), expectedCaughtCount);

        const GenPool::Handle invalidGenerationHandle { 0, 1 };

        EXPECT_EQ(hotPool.Get(invalidGenerationHandle), nullptr);
        EXPECT_EQ(hotAndColdPool.Get(invalidGenerationHandle), nullptr);
        EXPECT_EQ(hotAndColdPool.GetCold(invalidGenerationHandle), nullptr);

        // Should have received no assert
        EXPECT_EQ(catcher.GetCaughtMessages().size(), expectedCaughtCount);

        const GenPool::Handle outOfBoundsHandle { static_cast<u16>(hotPool.GetSize()), 0 };

        EXPECT_EQ(hotPool.Get(outOfBoundsHandle), nullptr);
        expectedCaughtCount++;

        EXPECT_EQ(hotAndColdPool.Get(outOfBoundsHandle), nullptr);
        expectedCaughtCount++;

        // Out of bounds should trigger assert
        EXPECT_EQ(catcher.GetCaughtMessages().size(), expectedCaughtCount);
    }

    TEST(GenerationalPool, Allocate)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        u32 expectedAssertCount = 0;

        GenerationalPool<u32, u32> pool;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        const GenPool::Handle firstHandle = pool.Allocate();
        EXPECT_NE(firstHandle, GenPool::kInvalidHandle);
        EXPECT_EQ(firstHandle.m_index, 0);
        EXPECT_EQ(firstHandle.m_generation, 0);

        for (u32 i = 1; i < pool.GetSize(); i++)
        {
            const GenPool::Handle handle = pool.Allocate();
            EXPECT_EQ(handle.m_index, i);
            EXPECT_EQ(handle.m_generation, 0);
        }

        EXPECT_EQ(catcher.GetCaughtMessages().size(), expectedAssertCount);

        // Fill to max size

        for (u32 i = pool.GetSize(); i < (1 << 16); i++)
        {
            const GenPool::Handle handle = pool.Allocate();
            EXPECT_EQ(handle.m_index, i);
            EXPECT_EQ(handle.m_generation, 0);
        }

        EXPECT_EQ(catcher.GetCaughtMessages().size(), expectedAssertCount);

        // Any additional allocation triggers an assert and returns an invalid handle
        {
            const GenPool::Handle handle = pool.Allocate();
            expectedAssertCount++;
            EXPECT_EQ(handle, GenPool::kInvalidHandle);
            EXPECT_EQ(catcher.GetCaughtMessages().size(), expectedAssertCount);
        }
    }
}
