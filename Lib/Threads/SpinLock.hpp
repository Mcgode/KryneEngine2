/**
 * @file
 * @author Max Godefroy
 * @date 03/07/2022.
 */

#pragma once

#include <atomic>
#include <Threads/HelperFunctions.hpp>

namespace KryneEngine
{
    /// @see https://rigtorp.se/spinlock/
    struct SpinLock
    {
    public:
        inline void Lock() noexcept
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

        inline void Unlock() noexcept
        {
            m_lock.store(false, std::memory_order_release);
        }

        [[nodiscard]] inline bool TryLock() noexcept
        {
            // First do a relaxed load to check if lock is free in order to prevent
            // unnecessary cache misses if someone does while(!try_lock())
            return !m_lock.load(std::memory_order_relaxed) &&
                   !m_lock.exchange(true, std::memory_order_acquire);
        }

        [[nodiscard]] inline bool IsLocked() const noexcept
        {
            return m_lock.load(std::memory_order_relaxed);
        }

    private:
        std::atomic<bool> m_lock = false;

        using LockGuardT = Threads::SyncLockGuard<SpinLock, &SpinLock::Lock, &SpinLock::Unlock>;

    public:
        [[nodiscard]] LockGuardT&& AutoLock() noexcept
        {
            return std::move(LockGuardT(this));
        }
    };
} // KryneEngine