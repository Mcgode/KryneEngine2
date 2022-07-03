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
                return *this;
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
            m_spinlock.Lock();
        }

        void ManualUnlock()
        {
            m_spinlock.Unlock();
        }

        [[nodiscard]] Lock&& AutoLock()
        {
            return eastl::move(Lock(this));
        }

    private:
        SpinLock m_spinlock {};
    };
}