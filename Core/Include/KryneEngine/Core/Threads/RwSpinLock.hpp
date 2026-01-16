/**
 * @file
 * @author Max Godefroy
 * @date 16/01/2026.
 */

#pragma once

#include <atomic>
#include "KryneEngine/Core/Threads/HelperFunctions.hpp"

namespace KryneEngine
{
    /**
     * @brief A fast and simple spinning RW mutex, ideal for low-contention scenarios where performance is crucial.
     *
     * @details
     * Based on Folly implementation: https://github.com/facebook/folly/blob/main/folly/synchronization/RWSpinLock.h
     * Do know that the lock is unfair: a writer can be stuck waiting for readers to be done.
     */
    class RwSpinLock
    {
    public:
        RwSpinLock() = default;

        void ReadLock() noexcept;
        [[nodiscard]] bool TryReadLock() noexcept;
        void ReadUnlock() noexcept;

        void WriteLock() noexcept;
        [[nodiscard]] bool TryWriteLock() noexcept;
        void WriteUnlock() noexcept;

    private:
        static constexpr u32 kWriterFlag = 1 << 0;
        static constexpr u32 kReadersOne = 1 << 1;

        static constexpr size_t kCpuYieldSpinCount = 64;
        static constexpr size_t kThreadYieldSpinCount = 96;

        std::atomic<u32> m_spinValue { 0 };

    public:
        using ReadLockGuardT = Threads::SyncLockGuard<RwSpinLock, &RwSpinLock::ReadLock, &RwSpinLock::ReadUnlock>;
        using WriteLockGuardT = Threads::SyncLockGuard<RwSpinLock, &RwSpinLock::WriteLock, &RwSpinLock::WriteUnlock>;

        ReadLockGuardT AutoReadLock() noexcept { return ReadLockGuardT(this); }
        WriteLockGuardT AutoWriteLock() noexcept { return WriteLockGuardT(this); }
    };
}