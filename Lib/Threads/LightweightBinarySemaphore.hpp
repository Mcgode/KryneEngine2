/**
 * @file
 * @author Max Godefroy
 * @date 03/07/2022.
 */

#pragma once

#include <Common/KETypes.hpp>
#include "SpinLock.hpp"

namespace KryneEngine
{
    struct LightweightBinarySemaphore
    {
    public:
        inline void Signal()
        {
            m_spinlock.Unlock();
        }

        inline void Wait()
        {
            m_spinlock.Lock();
        }

        inline bool IsLocked()
        {
            return m_spinlock.IsLocked();
        }

        inline bool TryWait()
        {
            return m_spinlock.TryLock();
        }

    private:
        SpinLock m_spinlock {};
    };
}