/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include "LightweightMutex.hpp"

namespace KryneEngine
{
    static constexpr tracy::SourceLocationData lightweightMutexSourceLocationData {
        "LightweightMutex",
        nullptr,
        __FILE__,
        __LINE__,
        0
    };

    LightweightMutex::LightweightMutex(u32 _spinCount)
        : m_spinCount(_spinCount)
        , m_ctx(&lightweightMutexSourceLocationData)
    {}

    void LightweightMutex::ManualLock()
    {
        m_ctx.BeforeLock();

        if (m_spinLock.TryLock(m_spinCount))
        {
            m_systemMutex.lock();
            m_acquiredSpinLock = true;
        }
        else
        {
            m_systemMutex.lock();
        }

        m_ctx.AfterLock();
    }

    bool LightweightMutex::TryLock()
    {
        if (m_spinLock.TryLock())
        {
            if (m_systemMutex.try_lock())
            {
                m_acquiredSpinLock = true;
                return true;
            }
            m_spinLock.Unlock();
        }
        return false;
    }

    void LightweightMutex::ManualUnlock()
    {
        if (m_acquiredSpinLock)
        {
            m_acquiredSpinLock = false;
            m_spinLock.Unlock();
        }
        m_systemMutex.unlock();

        m_ctx.AfterUnlock();
    }
}
