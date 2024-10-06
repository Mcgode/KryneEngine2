/**
* @file
* @author Max Godefroy
* @date 6/10/2024.
 */

#include "SpinLock.hpp"

namespace KryneEngine
{
    void SpinLock::Lock() noexcept
    {
        for (;;)
        {
            // Optimistically assume the lock is free on the first try
            if (!m_lock.exchange(true, std::memory_order_acquire))
            {
                return;
            }

            // Wait for lock to be released without generating cache misses
            while (m_lock.load(std::memory_order_relaxed))
            {
                // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
                // hyper-threads
                Threads::CpuYield();
            }
        }
    }

    void SpinLock::Unlock() noexcept
    {
        m_lock.store(false, std::memory_order_release);
    }

    bool SpinLock::TryLock() noexcept
    {
        // First do a relaxed load to check if lock is free in order to prevent
        // unnecessary cache misses if someone does while(!try_lock())
        return !m_lock.load(std::memory_order_relaxed) &&
               !m_lock.exchange(true, std::memory_order_acquire);
    }

    bool SpinLock::IsLocked() const noexcept
    {
        return m_lock.load(std::memory_order_relaxed);
    }
}