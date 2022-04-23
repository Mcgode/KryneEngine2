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

    class FiberThread
    {
    public:
        explicit FiberThread(FibersManager *_fiberManager, u16 _threadIndex);

        virtual ~FiberThread();

        using ThreadIndex = u16;
        [[nodiscard]] static ThreadIndex GetCurrentFiberThreadIndex();
        [[nodiscard]] static bool IsFiberThread();

    private:
        bool m_shouldStop = false;
        std::thread m_thread;

        static constexpr u32 kRetrieveSpinCount = 50;
        static constexpr bool kSleepToSavePerformance = true;

        static thread_local ThreadIndex sThreadIndex;
        static thread_local bool sIsThread;
    };
} // KryneEngine