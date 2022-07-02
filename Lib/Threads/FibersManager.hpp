/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include <EASTL/array.h>
#include <moodycamel/concurrentqueue.h>
#include <Threads/FiberThread.hpp>
#include <Threads/FiberJob.hpp>
#include <Threads/FiberTls.hpp>

namespace KryneEngine
{
    class FibersManager
    {
        friend FiberThread;

    public:
        explicit FibersManager(u16 _fiberThreadCount);

        ~FibersManager();

        [[nodiscard]] static FibersManager* GetInstance();

        [[nodiscard]] static u16 GetFibersCount()
        {
            auto* mgr = GetInstance();
            if (!Verify(mgr != nullptr))
            {
                return 0;
            }
            return mgr->GetFiberThreadCount();
        }

        [[nodiscard]] u16 GetFiberThreadCount() const { return m_fiberThreads.Size(); }

    private:
        using JobType = FiberJob*;
        using JobQueue = moodycamel::ConcurrentQueue<JobType>;
        static constexpr u8 kJobQueuesCount = FiberJob::PriorityType::kJobPriorityTypes;
        eastl::array<JobQueue, kJobQueuesCount> m_jobQueues;

        using JobProducerTokenArray = eastl::array<moodycamel::ProducerToken, kJobQueuesCount>;
        FiberTls<JobProducerTokenArray> m_jobProducerTokens;

        using JobConsumerTokenArray = eastl::array<moodycamel::ConsumerToken, kJobQueuesCount>;
        FiberTls<JobConsumerTokenArray> m_jobConsumerTokens;

        DynamicArray<FiberThread> m_fiberThreads;

        static thread_local FibersManager* sManager;

        bool _RetrieveNextJob(JobType &job_, u16 _fiberIndex);
    };
} // KryneEngine