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
        explicit LightweightMutex(u32 _threadYieldThreshold = 1'000, u32 _systemMutexThreshold = 0);

        void ManualLock();

        void ManualUnlock();

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