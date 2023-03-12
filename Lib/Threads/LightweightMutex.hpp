/**
 * @file
 * @author Max Godefroy
 * @date 03/07/2022.
 */

#pragma once

#include <mutex>
#include <Threads/SpinLock.hpp>
#include <EASTL/algorithm.h>

namespace KryneEngine
{
    struct LightweightMutex
    {
    public:
        explicit LightweightMutex(u32 _threadYieldThreshold = 1'000,
                                  u32 _systemMutexThreshold = 0)
            : m_lock(false)
            , m_threadYieldThreshold(_threadYieldThreshold)
            , m_systemMutexThreshold(_systemMutexThreshold)
        {}

        void ManualLock()
        {
            u32 i = 0;
            for (;;)
            {
                // Optimistically assume the lock is free on the first try
                if (!m_lock.exchange(true, std::memory_order_acquire))
                {
                    if (m_systemMutexThreshold != 0)
                    {
                        m_systemMutex.lock();
                        systemMutexLocked = true;
                    }
                    return;
                }

                // Wait for lock to be released without generating cache misses
                do
                {
                    // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
                    // hyper-threads
                    Threads::CpuYield();

                    if (m_systemMutexThreshold != 0 && i >= m_systemMutexThreshold)
                    {
                        i = 0;
                        m_systemMutex.lock();
                        systemMutexLocked = true;
                    }
                    else if (i % m_threadYieldThreshold == 0)
                    {
                        std::this_thread::yield();
                    }
                }
                while (m_lock.load(std::memory_order_relaxed));
            }
        }

        void ManualUnlock()
        {
            if (systemMutexLocked)
            {
                systemMutexLocked = false;
                m_systemMutex.unlock();
            }
            m_lock.store(false, std::memory_order_release);
        }

    private:
        std::atomic<bool> m_lock ;
        u32 m_threadYieldThreshold;
        u32 m_systemMutexThreshold;
        std::mutex m_systemMutex;
        bool systemMutexLocked = false;

    public:
        using LockGuardT = Threads::SyncLockGuard<LightweightMutex, &LightweightMutex::ManualLock, &LightweightMutex::ManualUnlock>;

        [[nodiscard]] LockGuardT AutoLock()
        {
            return LockGuardT(this);
        }
    };
}