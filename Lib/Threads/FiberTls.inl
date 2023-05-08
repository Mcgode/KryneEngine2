/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#include "FiberTls.hpp"
#include <Threads/FibersManager.hpp>

namespace KryneEngine
{
    template<class T>
    inline void FiberTls<T>::Init(const FibersManager *_fibersManager, const T &_value)
    {
        m_array.Resize(_fibersManager->GetFiberThreadCount());
        m_array.SetAll(_value);
    }

    template<class T>
    template<typename F>
    void FiberTls<T>::InitFunc(const FibersManager *_fibersManager, F _initFunction)
    {
        m_array.Resize(_fibersManager->GetFiberThreadCount());

        for (auto& localFiberValue: m_array)
        {
            _initFunction(localFiberValue);
        }
    }

    template<class T>
    inline T& FiberTls<T>::Load()
    {
        KE_ASSERT(FiberThread::IsFiberThread());

        return Load(FiberThread::GetCurrentFiberThreadIndex());
    }
} // KryneEngine