/**
 * @file
 * @author Max Godefroy
 * @date 16/01/2026.
 */

#include "KryneEngine/Core/Threads/RwSpinLock.hpp"

#include "KryneEngine/Core/Common/Assert.hpp"

namespace KryneEngine
{
    void RwSpinLock::ReadLock() noexcept
    {
        size_t spinCount = 0;
        while (!TryReadLock())
        {
            spinCount++;
            if (spinCount >= kThreadYieldSpinCount)
                std::this_thread::yield();
            else if (spinCount >= kCpuYieldSpinCount)
                Threads::CpuYield();
        }
    }

    bool RwSpinLock::TryReadLock() noexcept
    {
        // We use fetch_add instead of CAS as it has better performance in hit cases (which are expected to be frequent)
        const u32 value = m_spinValue.fetch_add(kReadersOne, std::memory_order_acquire);
        if ((value & kWriterFlag) != 0) [[unlikely]]
        {
            m_spinValue.fetch_sub(kReadersOne, std::memory_order_release);
            return false;
        }
        return true;
    }

    void RwSpinLock::ReadUnlock() noexcept
    {
        const u32 value = m_spinValue.fetch_sub(kReadersOne, std::memory_order_release);
        KE_ASSERT(value >= kReadersOne && (value & kWriterFlag) == 0);
    }

    void RwSpinLock::WriteLock() noexcept
    {
        size_t spinCount = 0;
        while (!TryWriteLock())
        {
            spinCount++;
            if (spinCount >= kThreadYieldSpinCount)
                std::this_thread::yield();
            else if (spinCount >= kCpuYieldSpinCount)
                Threads::CpuYield();
        }
    }

    bool RwSpinLock::TryWriteLock() noexcept
    {
        u32 expected = 0;
        return m_spinValue.compare_exchange_strong(
            expected,
            kWriterFlag,
            std::memory_order::acq_rel);
    }

    void RwSpinLock::WriteUnlock() noexcept
    {
        const u32 value =m_spinValue.fetch_and(~kWriterFlag, std::memory_order_release);
        KE_ASSERT(value == kWriterFlag);
    }
} // namespace KryneEngine