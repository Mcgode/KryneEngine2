/**
 * @file
 * @author Max Godefroy
 * @date 16/11/2022.
 */

#pragma once

#include <atomic>
#include <Common/Assert.hpp>
#include <Common/Types.hpp>
#include <Threads/HelperFunctions.hpp>

namespace KryneEngine
{
    /**
     * @brief A Fiber-safe busy semaphore.
     */
    class LightweightSemaphore
    {
    public:
        explicit LightweightSemaphore(u32 _count, u32 _spinCount = 1'000);

        inline void Signal(u32 _count) noexcept { m_count += _count; }

        inline void SignalOnce() noexcept { Signal(1); }

        bool TryWait() noexcept;

        void Wait() noexcept;

    private:
        std::atomic<u32> m_count;
        u32 m_yieldSpinCount;

        using LockGuardT = Threads::SyncLockGuard<LightweightSemaphore, &LightweightSemaphore::Wait, &LightweightSemaphore::SignalOnce>;

    public:
        [[nodiscard]] LockGuardT AutoLock()
        {
            return LockGuardT(this);
        }
    };
}