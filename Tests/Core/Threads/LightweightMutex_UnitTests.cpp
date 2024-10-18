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
        mutex.ManualUnlock(); // Not tested yet, but needed to avoid destroying busy mutex
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

        mutex.ManualUnlock();
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
        SpinLock syncLock;

        syncLock.Lock();

        bool finished = false;
        std::thread unlockThread([&](){
            EXPECT_TRUE(mutex.TryLock());
            syncLock.Unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            finished = true;
            mutex.ManualUnlock();
        });

        syncLock.Lock();

        mutex.ManualLock();

        EXPECT_TRUE(finished);
        EXPECT_TRUE(unlockThread.joinable());
        EXPECT_FALSE(mutex.TryLock());
        EXPECT_TRUE(syncLock.IsLocked());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        unlockThread.join();
        mutex.ManualUnlock();
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
        SpinLock syncLock;

        syncLock.Lock();

        bool finished = false;
        std::thread unlockThread([&](){
            const auto lock = mutex.AutoLock();
            syncLock.Unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            finished = true;
        });

        syncLock.Lock();
        {
            const auto lock = mutex.AutoLock();
            EXPECT_TRUE(unlockThread.joinable());
            EXPECT_TRUE(finished);
            EXPECT_FALSE(mutex.TryLock());
        }

        EXPECT_TRUE(mutex.TryLock());
        EXPECT_FALSE(mutex.TryLock());
        EXPECT_TRUE(syncLock.IsLocked());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        unlockThread.join();
        mutex.ManualUnlock();
        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }
}
