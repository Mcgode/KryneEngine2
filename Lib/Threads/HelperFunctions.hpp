/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include <thread>
#include <Common/KETypes.hpp>

namespace KryneEngine::Threads
{
    bool SetThreadHardwareAffinity(std::thread& _thread, u32 _coreIndex);

    bool DisableThreadSignals();

    inline void CpuYield()
    {
#if defined(__x86_64__) || defined(__i386__) || defined(_WIN32)
        _mm_pause();
#elif defined(__arm__)
        __yield()
#else
#warning No CPU pause/yield instruction
#endif
    }

    template<class SyncPrimitive, void (SyncPrimitive::*LockMethod)(), void (SyncPrimitive::*UnlockMethod)()>
    struct SyncLockGuard
    {
        explicit SyncLockGuard(SyncPrimitive* _primitive)
        {
            m_syncPrimitive = _primitive;
            if (m_syncPrimitive != nullptr)
            {
                (m_syncPrimitive->*LockMethod)();
            }
        }

        SyncLockGuard(const SyncLockGuard& _other) = delete;
        SyncLockGuard& operator=(const SyncLockGuard& _other) = delete;

        SyncLockGuard(SyncLockGuard&& _other) noexcept
        {
            m_syncPrimitive = _other.m_syncPrimitive;
            _other.m_syncPrimitive = nullptr;
        }

        ~SyncLockGuard()
        {
            if (m_syncPrimitive != nullptr)
            {
                (m_syncPrimitive->*UnlockMethod)();
            }
        }

    private:
        SyncPrimitive* m_syncPrimitive = nullptr;
    };
}
