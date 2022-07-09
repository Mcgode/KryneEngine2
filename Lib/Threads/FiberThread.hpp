/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include <thread>
#include <Common/KETypes.hpp>

namespace KryneEngine
{
    class FibersManager;
    class FiberJob;

    class FiberThread
    {
    public:
        explicit FiberThread(FibersManager *_fiberManager, u16 _threadIndex);

        virtual ~FiberThread();

        using ThreadIndex = u16;
        [[nodiscard]] static ThreadIndex GetCurrentFiberThreadIndex();
        [[nodiscard]] static bool IsFiberThread();

        void SwitchToNextJob(FibersManager *_manager, FiberJob *_currentJob, FiberJob *_nextJob = nullptr);

    private:
        bool m_shouldStop = false;
        std::thread m_thread;

        static constexpr u32 kRetrieveSpinCountBeforeThreadYield = 50;

        static thread_local ThreadIndex sThreadIndex;
        static thread_local bool sIsThread;

        FiberJob *_TryRetrieveNextJob(FibersManager *_manager, u16 _threadIndex, bool _busyWait);
    };
} // KryneEngine