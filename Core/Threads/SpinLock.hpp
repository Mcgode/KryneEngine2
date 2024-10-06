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
        void Lock() noexcept;

        void Unlock() noexcept;

        [[nodiscard]] bool TryLock() noexcept;
        [[nodiscard]] bool TryLock(u32 _spinCount) noexcept;

        [[nodiscard]] bool IsLocked() const noexcept;

    private:
        std::atomic<bool> m_lock = false;

        using LockGuardT = Threads::SyncLockGuard<SpinLock, &SpinLock::Lock, &SpinLock::Unlock>;

    public:
        [[nodiscard]] LockGuardT AutoLock() noexcept
        {
            return LockGuardT(this);
        }
    };
} // KryneEngine