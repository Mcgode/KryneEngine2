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
                FibersManager::JobType job;
                bool foundJob = false;

                for (u32 i = 0; i < kRetrieveSpinCount; i++)
                {
                    foundJob = FibersManager::GetInstance()->_RetrieveNextJob(job);
                    if (foundJob)
                    {
                        break;
                    }
                }

                if (foundJob)
                {
                    // TODO: run job
                }
                else if (kSleepToSavePerformance)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(0));
                }
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