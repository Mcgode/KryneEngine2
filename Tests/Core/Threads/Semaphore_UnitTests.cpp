/**
* @file
* @author Max Godefroy
* @date 27/09/2024.
*/

#include <gtest/gtest.h>

#include <Threads/Semaphore.hpp>
#include <Utils/AssertUtils.hpp>

namespace KryneEngine::Tests
{
    TEST(BusySpinSemaphore, TryWait)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        // TryWait binary semaphore
        {
            BusySpinSemaphore semaphore(1);

            EXPECT_TRUE(semaphore.TryWait());
            EXPECT_FALSE(semaphore.TryWait());
        }

        // TryWait non-binary semaphore
        {
            constexpr u32 count = 16;
            BusySpinSemaphore semaphore(count);

            for (u32 i = 0; i < count; i++)
            {
                EXPECT_TRUE(semaphore.TryWait());
            }
            EXPECT_FALSE(semaphore.TryWait());
        }

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(BusySpinSemaphore, Signal)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        BusySpinSemaphore semaphore(1);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_FALSE(semaphore.TryWait());

        semaphore.Signal(1);

        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_FALSE(semaphore.TryWait());

        semaphore.Signal(2);

        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_FALSE(semaphore.TryWait());

        semaphore.SignalOnce();

        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_FALSE(semaphore.TryWait());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(BusySpinSemaphore, Wait)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        BusySpinSemaphore semaphore(1);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_FALSE(semaphore.TryWait());

        // Even if thread creation is instant or blocking, the thread will wait for at least 1ms before signaling the
        // semaphore, making sure the parallel operations are executed in the correct order.
        std::thread signalThread0([&](){
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            semaphore.SignalOnce();
        });

        semaphore.Wait();
        EXPECT_TRUE(signalThread0.joinable());
        EXPECT_FALSE(semaphore.TryWait());

        std::thread signalThread1([&](){
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            semaphore.Signal(2);
        });

        semaphore.Wait();
        EXPECT_TRUE(signalThread1.joinable());
        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_FALSE(semaphore.TryWait());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        signalThread0.join();
        signalThread1.join();
        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(BusySpinSemaphore, AutoLock)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        BusySpinSemaphore semaphore(1);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_FALSE(semaphore.TryWait());

        // Even if thread creation is instant or blocking, the thread will wait for at least 1ms before signaling the
        // semaphore, making sure the parallel operations are executed in the correct order.
        std::thread signalThread0([&](){
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            semaphore.SignalOnce();
        });

        {
            const auto lock = semaphore.AutoLock();
            EXPECT_TRUE(signalThread0.joinable());
            EXPECT_FALSE(semaphore.TryWait());
        }
        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_FALSE(semaphore.TryWait());

        std::thread signalThread1([&](){
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            semaphore.Signal(2);
        });

        {
            const auto lock = semaphore.AutoLock();
            EXPECT_TRUE(signalThread1.joinable());
            EXPECT_TRUE(semaphore.TryWait());
            EXPECT_FALSE(semaphore.TryWait());
        }
        EXPECT_TRUE(semaphore.TryWait());
        EXPECT_FALSE(semaphore.TryWait());

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        signalThread0.join();
        signalThread1.join();
        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }
}
