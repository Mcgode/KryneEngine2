/**
 * @file
 * @author Max Godefroy
 * @date 03/07/2022.
 */

#pragma once

#include <ck_spinlock.h>
#include <EASTL/algorithm.h>

namespace KryneEngine
{
    struct LightweightMutex
    {
    public:
        struct Lock
        {
            friend LightweightMutex;

        public:
            Lock() = default;

            ~Lock()
            {
                if (m_mutex != nullptr)
                {
                    m_mutex->ManualUnlock();
                }
            }

            Lock(Lock& _other) = delete;
            Lock& operator=(Lock& _other) = delete;

            Lock(Lock&& _other) noexcept
                : m_mutex(_other.m_mutex)
            {
                _other.m_mutex = nullptr;
            }
            Lock& operator=(Lock&& _other) noexcept
            {
                m_mutex = _other.m_mutex;
                _other.m_mutex = nullptr;
            }

        private:
            explicit Lock(LightweightMutex* _mutex)
                : m_mutex(_mutex)
            {
                m_mutex->ManualLock();
            }

            LightweightMutex* m_mutex = nullptr;
        };

    public:
        void ManualLock()
        {
            ck_spinlock_lock(&m_spinlock);
        }

        void ManualUnlock()
        {
            ck_spinlock_unlock(&m_spinlock);
        }

        [[nodiscard]] Lock&& AutoLock()
        {
            return eastl::move(Lock(this));
        }

    private:
        ck_spinlock_t m_spinlock = CK_SPINLOCK_INITIALIZER;
    };
}