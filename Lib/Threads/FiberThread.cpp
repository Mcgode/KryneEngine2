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

#if CONTEXT_SWITCH_WINDOWS_FIBERS
            auto& context = _fiberManager->m_baseContexts.Load(_threadIndex);
            context.m_winFiber = ConvertThreadToFiber(nullptr);
#endif

            while (!m_shouldStop)
            {
                SwitchToNextJob(_fiberManager, nullptr);
                _fiberManager->_OnContextSwitched();
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

    void FiberThread::SwitchToNextJob(FibersManager *_manager, FiberJob *_currentJob, FiberJob *_nextJob)
    {
        const auto fiberIndex = GetCurrentFiberThreadIndex();

        if (_nextJob == nullptr)
        {
            _nextJob = _TryRetrieveNextJob(_manager, 0, _currentJob == nullptr);
        }

        // Happens when shutting down.
        if (_nextJob == nullptr && _currentJob == nullptr)
        {
            return;
        }

        _manager->m_nextJob.Load(fiberIndex) = _nextJob;

        auto* currentContext = _currentJob == nullptr
                ? &_manager->m_baseContexts.Load(fiberIndex)
                : _currentJob->m_context;
        auto* nextContext = _nextJob == nullptr
                ? &_manager->m_baseContexts.Load(fiberIndex)
                : _nextJob->m_context;
        Assert(nextContext != nullptr);
        currentContext->SwapContext(nextContext);
        _manager->_OnContextSwitched();
    }

    FiberJob *FiberThread::_TryRetrieveNextJob(FibersManager *_manager, u16 _threadIndex, bool _busyWait)
    {
        FibersManager::JobType job;

        u32 i = 0;
        do
        {
            if (FibersManager::GetInstance()->_RetrieveNextJob(job, _threadIndex))
            {
                break;
            }
            else if (i >= kRetrieveSpinCount && kSleepToSavePerformance)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(0));
                i = 0;
            }
            else
            {
                i++;
            }
        }
        while(!m_shouldStop && _busyWait);

        return m_shouldStop ? nullptr : job;
    }
} // KryneEngine