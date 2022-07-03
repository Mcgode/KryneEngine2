/**
 * @file
 * @author Max Godefroy
 * @date 03/07/2022.
 */

#pragma once

#include <ck_spinlock.h>
#include <Common/KETypes.hpp>

namespace KryneEngine
{
    struct LightweightBinarySemaphore
    {
    public:
        inline void Signal()
        {
            ck_spinlock_unlock(&m_spinlock);
        }

        inline void Wait()
        {
            ck_spinlock_lock(&m_spinlock);
        }

        inline bool IsLocked()
        {
            return ck_spinlock_locked(&m_spinlock);
        }

        inline bool TryWait()
        {
            return ck_spinlock_trylock(&m_spinlock);
        }

    private:
        ck_spinlock_t m_spinlock = CK_SPINLOCK_INITIALIZER;
    };
}