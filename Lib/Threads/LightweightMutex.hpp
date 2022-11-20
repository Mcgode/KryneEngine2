/**
 * @file
 * @author Max Godefroy
 * @date 03/07/2022.
 */

#pragma once

#include <Threads/SpinLock.hpp>
#include <EASTL/algorithm.h>

namespace KryneEngine
{
    struct LightweightMutex
    {
    public:
        void ManualLock()
        {
            m_spinlock.Lock();
        }

        void ManualUnlock()
        {
            m_spinlock.Unlock();
        }

    private:
        SpinLock m_spinlock {};

        using LockGuardT = Threads::SyncLockGuard<LightweightMutex, &LightweightMutex::ManualLock, &LightweightMutex::ManualUnlock>;

    public:
        [[nodiscard]] LockGuardT&& AutoLock()
        {
            return eastl::move(LockGuardT(this));
        }
    };
}