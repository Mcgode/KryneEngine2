/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#include "FiberThread.hpp"

#include <Threads/FibersManager.hpp>
#include <Threads/HelperFunctions.hpp>

namespace KryneEngine
{
    thread_local FiberThread::ThreadIndex FiberThread::sThreadIndex = 0;
    thread_local bool FiberThread::sIsThread = false;

    FiberThread::FiberThread(FibersManager *_fiberManager, u16 _threadIndex)
    {
        m_thread = std::thread([=]()
        {
            Assert(Threads::DisableThreadSignals());

            FibersManager::sManager = _fiberManager;
            sThreadIndex = _threadIndex;
            sIsThread = true;

            while (!m_shouldStop)
            {

            }
        });

        Assert(Threads::SetThreadHardwareAffinity(m_thread, _threadIndex));
    }

    FiberThread::~FiberThread()
    {
        m_shouldStop = true;
        m_thread.join();
    }

    FiberThread::ThreadIndex FiberThread::GetCurrentFiberThreadIndex()
    {
        return sThreadIndex;
    }

    bool FiberThread::IsFiberThread()
    {
        return sIsThread;
    }
} // KryneEngine