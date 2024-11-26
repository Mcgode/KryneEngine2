/**
* @file
* @author Max Godefroy
* @date 6/10/2024.
*/

#include "KryneEngine/Core/Threads/LightweightSemaphore.hpp"

namespace KryneEngine
{
    LightweightSemaphore::LightweightSemaphore(u32 _count, u32 _spinCount)
        : m_count(_count)
        , m_yieldSpinCount(_spinCount)
    {}

    bool LightweightSemaphore::TryWait() noexcept
    {
        u32 count;

        // Try to atomically decrease the counter while it is not zero.
        // Can be costly is there's a lot of contention, but ensures that the counter never goes below 0
        while ((count = m_count.load(std::memory_order::relaxed)) > 0)
        {
            if (m_count.compare_exchange_strong(count, count - 1, std::memory_order_acquire))
            {
                return true;
            }
            std::atomic_signal_fence(std::memory_order_acquire);
        }

        return false;
    }

    void LightweightSemaphore::Wait() noexcept
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

    LightweightBinarySemaphore::LightweightBinarySemaphore(u32 _spinCount)
        : m_yieldSpinCount(_spinCount)
    {
    }

    void LightweightBinarySemaphore::Wait() noexcept
    {
        while (!m_spinLock.TryLock(m_yieldSpinCount))
        {
            std::this_thread::yield();
        }
    }
}
