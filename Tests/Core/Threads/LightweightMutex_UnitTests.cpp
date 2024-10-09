/**
* @file
* @author Max Godefroy
* @date 9/10/2024.
 */

#include <gtest/gtest.h>

#include <Threads/LightweightMutex.hpp>
#include <Utils/AssertUtils.hpp>

namespace KryneEngine::Tests
{
    TEST(LightweightMutex, TryLock)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        LightweightMutex mutex;

        EXPECT_TRUE(mutex.TryLock());
        EXPECT_FALSE(mutex.TryLock());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(LightweightMutex, ManualUnlock)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        LightweightMutex mutex;

        EXPECT_TRUE(mutex.TryLock());
        EXPECT_FALSE(mutex.TryLock());

        mutex.ManualUnlock();
        EXPECT_TRUE(mutex.TryLock());
        EXPECT_FALSE(mutex.TryLock());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(LightweightMutex, ManualLock)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        LightweightMutex mutex;

        EXPECT_TRUE(mutex.TryLock());
        EXPECT_FALSE(mutex.TryLock());

        bool finished = false;
        // Even if thread creation is instant or blocking, the thread will wait for at least 1ms before unlocking the
        // spinlock, making sure the parallel operations are executed in the correct order.
        std::thread unlockThread([&](){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            mutex.ManualUnlock();
            finished = true;
        });

        mutex.ManualLock();
        EXPECT_TRUE(unlockThread.joinable());
        EXPECT_TRUE(finished);
        EXPECT_FALSE(mutex.TryLock());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        unlockThread.join();
        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(LightweightMutex, AutoLock)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        LightweightMutex mutex;

        EXPECT_TRUE(mutex.TryLock());
        EXPECT_FALSE(mutex.TryLock());

        bool finished = false;
        // Even if thread creation is instant or blocking, the thread will wait for at least 1ms before unlocking the
        // spinlock, making sure the parallel operations are executed in the correct order.
        std::thread unlockThread([&](){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            mutex.ManualUnlock();
            finished = true;
        });

        {
            const auto lock = mutex.AutoLock();
            EXPECT_TRUE(unlockThread.joinable());
            EXPECT_TRUE(finished);
            EXPECT_FALSE(mutex.TryLock());
        }

        EXPECT_TRUE(mutex.TryLock());
        EXPECT_FALSE(mutex.TryLock());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        unlockThread.join();
        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }
}
