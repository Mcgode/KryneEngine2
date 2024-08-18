/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include "LightweightMutex.hpp"

namespace KryneEngine
{
    static constexpr tracy::SourceLocationData lightweightMutexSourceLocationData {
        "LightweightMutex",
        nullptr,
        __FILE__,
        __LINE__,
        0
    };

    LightweightMutex::LightweightMutex(u32 _threadYieldThreshold, u32 _systemMutexThreshold)
        : m_lock(false)
        , m_threadYieldThreshold(_threadYieldThreshold)
        , m_systemMutexThreshold(_systemMutexThreshold)
        , m_ctx(&lightweightMutexSourceLocationData)
    {}

    void LightweightMutex::ManualLock()
    {
        m_ctx.BeforeLock();

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
                break;
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

        m_ctx.AfterLock();
    }

    void LightweightMutex::ManualUnlock()
    {
        if (systemMutexLocked)
        {
            systemMutexLocked = false;
            m_systemMutex.unlock();
        }
        m_lock.store(false, std::memory_order_release);

        m_ctx.AfterUnlock();
    }
}