/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include <Common/KETypes.hpp>
#include <EASTL/array.h>
#include <moodycamel/concurrentqueue.h>
#include <Threads/FiberThread.hpp>
#include <Common/Arrays.hpp>

namespace KryneEngine
{
    enum class FibersJobPriority: u8
    {
        Low,
        Medium,
        High,
        Count
    };

    class FibersManager
    {
        friend FiberThread;

    public:
        explicit FibersManager(u16 _fiberThreadCount);

        [[nodiscard]] static FibersManager* GetInstance();

        [[nodiscard]] static u16 GetFibersCount()
        {
            auto* mgr = GetInstance();
            if (!Verify(mgr != nullptr))
            {
                return 0;
            }
            return mgr->m_fiberThreads.Size();
        }

    private:
        using JobType = void*;
        using JobQueue = moodycamel::ConcurrentQueue<JobType>;
        static constexpr u8 kJobQueuesCount = static_cast<u8>(FibersJobPriority::Count) + 1;
        eastl::array<JobQueue, kJobQueuesCount> m_jobQueues;

        DynamicArray<FiberThread> m_fiberThreads;

        static thread_local FibersManager* sManager;

        bool _RetrieveNextJob(JobType &job_);
    };
} // KryneEngine