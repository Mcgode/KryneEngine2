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
    inline void FiberTls<T>::Init()
    {
        m_array.Resize(FibersManager::GetFibersCount());
    }

    template<class T>
    inline T& FiberTls<T>::Load()
    {
        Assert(FiberThread::IsFiberThread());

        return Load(FiberThread::GetCurrentFiberThreadIndex());
    }
} // KryneEngine