/**
 * @file
 * @author Max Godefroy
 * @date 06/10/2024.
 */

#include <gtest/gtest.h>
#include <KryneEngine/Core/Threads/SpinLock.hpp>

#include "Utils/AssertUtils.hpp"

namespace KryneEngine::Tests
{
    TEST(SpinLock, TryLock)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        // TryLock() should only work on its first call
        {
            SpinLock spinLock;
            EXPECT_TRUE(spinLock.TryLock());
            EXPECT_FALSE(spinLock.TryLock());
            for (u32 i = 0; i < 1'048'756; i++)
            {
                EXPECT_FALSE(spinLock.TryLock());
            }
        }

        // Scoped, and should work on any individual spinlock
        {
            SpinLock spinLock0;
            EXPECT_TRUE(spinLock0.TryLock());

            SpinLock spinLock1;
            EXPECT_TRUE(spinLock1.TryLock());

            SpinLock spinLock2;
            EXPECT_TRUE(spinLock2.TryLock());
        }

        {
            SpinLock spinLock;
            EXPECT_TRUE(spinLock.TryLock(1'024));
            EXPECT_FALSE(spinLock.TryLock(1'024));
        }

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(SpinLock, IsLocked)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        SpinLock spinLock;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_FALSE(spinLock.IsLocked());

        EXPECT_TRUE(spinLock.TryLock());
        EXPECT_TRUE(spinLock.IsLocked());

        EXPECT_FALSE(spinLock.TryLock());
        EXPECT_TRUE(spinLock.IsLocked());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(SpinLock, Unlock)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        SpinLock spinLock;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_FALSE(spinLock.IsLocked());

        EXPECT_TRUE(spinLock.TryLock());
        EXPECT_TRUE(spinLock.IsLocked());

        EXPECT_FALSE(spinLock.TryLock());
        EXPECT_TRUE(spinLock.IsLocked());

        spinLock.Unlock();
        EXPECT_FALSE(spinLock.IsLocked());

        EXPECT_TRUE(spinLock.TryLock());
        EXPECT_TRUE(spinLock.IsLocked());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(SpinLock, Lock)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        SpinLock spinLock;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_FALSE(spinLock.IsLocked());

        EXPECT_TRUE(spinLock.TryLock());
        EXPECT_TRUE(spinLock.IsLocked());

        // Even if thread creation is instant or blocking, the thread will wait for at least 1ms before unlocking the
        // spinlock, making sure the parallel operations are executed in the correct order.
        std::thread unlockThread([&](){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            spinLock.Unlock();
        });

        spinLock.Lock();
        EXPECT_TRUE(unlockThread.joinable());
        EXPECT_TRUE(spinLock.IsLocked());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        unlockThread.join();
        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(SpinLock, AutoLock)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        SpinLock spinLock;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_FALSE(spinLock.IsLocked());

        EXPECT_TRUE(spinLock.TryLock());
        EXPECT_TRUE(spinLock.IsLocked());

        // Even if thread creation is instant or blocking, the thread will wait for at least 1ms before unlocking the
        // spinlock, making sure the parallel operations are executed in the correct order.
        std::thread unlockThread([&](){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            spinLock.Unlock();
        });

        {
            const auto lock = spinLock.AutoLock();
            EXPECT_TRUE(unlockThread.joinable());
            EXPECT_TRUE(spinLock.IsLocked());
        }
        EXPECT_FALSE(spinLock.IsLocked());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        unlockThread.join();
        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }
}
