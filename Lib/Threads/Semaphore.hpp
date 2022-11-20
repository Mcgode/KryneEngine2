/**
 * @file
 * @author Max Godefroy
 * @date 16/11/2022.
 */

#pragma once

#include <atomic>
#include <Common/Assert.hpp>
#include <Common/KETypes.hpp>
#include <Threads/HelperFunctions.hpp>

namespace KryneEngine
{
    /**
     * @brief A Fiber-safe busy semaphore.
     */
    class BusySpinSemaphore
    {
    public:
        explicit BusySpinSemaphore(u32 _count, u32 _spinCount = 1'000)
            : m_count(_count)
            , m_yieldSpinCount(_spinCount)
        {}

        inline void Signal(u32 _count) noexcept
        {
            m_count += _count;
        }

        inline void SignalOnce() noexcept { Signal(1); }

        inline bool TryWait() noexcept
        {
            u32 count;

            // Try to atomically decrease the counter while it is not zero.
            // Can be costly is there's a lot of contention, but ensures that the counter never goes below 0
            while ((count = m_count.load(std::memory_order::relaxed)) > 0)
            {
                if (m_count.compare_exchange_strong(count, count - 1, std::memory_order::memory_order_acquire))
                {
                    return true;
                }
                std::atomic_signal_fence(std::memory_order_acquire);
            }

            return false;
        }

        inline void Wait() noexcept
        {
            u32 spinCount = 0;

            for (;;)
            {
                // Try wait, if fails do spinlock
                if (TryWait())
                {
                    return;
                }

                // Wait for lock to be released without generating cache misses
                while (m_count.load(std::memory_order_relaxed) == 0)
                {
                    if (++spinCount < m_yieldSpinCount)
                    {
                        // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
                        // hyper-threads
                        Threads::CpuYield();
                    }
                    else
                    {
                        // Yield thread.
                        std::this_thread::yield();

                        spinCount = 0;
                    }
                }
            }
        }

    private:
        std::atomic<u32> m_count;
        u32 m_yieldSpinCount;

        using LockGuardT = Threads::SyncLockGuard<BusySpinSemaphore, &BusySpinSemaphore::Wait, &BusySpinSemaphore::SignalOnce>;

    public:
        [[nodiscard]] LockGuardT&& AutoLock()
        {
            return std::move(LockGuardT(this));
        }
    };
}