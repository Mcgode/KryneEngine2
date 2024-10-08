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

        while (!m_spinLock.TryLock(m_spinCount))
        {
            const auto lock = m_internalStatusSpinLock.AutoLock();

            if (!m_spinLock.TryLock())
            {
                m_lockedSystemMutex = true;
                m_systemMutex.lock();
            }
        }

        m_ctx.AfterLock();
    }

    void LightweightMutex::ManualUnlock()
    {
        {
            const auto lock = m_internalStatusSpinLock.AutoLock();
            if (m_lockedSystemMutex)
            {
                m_systemMutex.unlock();
                m_lockedSystemMutex = false;
            }
            m_spinLock.Unlock();
        }

        m_ctx.AfterUnlock();
    }
}
