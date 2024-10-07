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
            m_systemMutex.lock();
        }

        m_ctx.AfterLock();
    }

    void LightweightMutex::ManualUnlock()
    {
        m_systemMutex.try_lock(); // Cannot unlock a mutex which hasn't been acquired. Makes sure the mutex is locked
        m_systemMutex.unlock();
        m_spinLock.Unlock();

        m_ctx.AfterUnlock();
    }
}
