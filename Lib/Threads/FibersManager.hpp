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
        using JobType = FiberJob*;

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

        [[nodiscard]] FiberJob* GetCurrentJob();

        void YieldJob();

    private:
        using JobQueue = moodycamel::ConcurrentQueue<JobType>;
        static constexpr u8 kJobQueuesCount = FiberJob::PriorityType::kJobPriorityTypes;
        eastl::array<JobQueue, kJobQueuesCount> m_jobQueues;

        using JobProducerTokenArray = eastl::array<moodycamel::ProducerToken, kJobQueuesCount>;
        FiberTls<JobProducerTokenArray> m_jobProducerTokens;

        using JobConsumerTokenArray = eastl::array<moodycamel::ConsumerToken, kJobQueuesCount>;
        FiberTls<JobConsumerTokenArray> m_jobConsumerTokens;

        DynamicArray<FiberThread> m_fiberThreads;

        FiberTls<FiberJob*> m_currentJobs;
        FiberTls<FiberContext> m_contexts;

        static thread_local FibersManager* sManager;

        bool _RetrieveNextJob(JobType &job_, u16 _fiberIndex);

    private:
        static constexpr u64 kSmallStackSize = 64 * 1024; // 64 KiB
        static constexpr u16 kSmallStackCount = 128;
        static constexpr u64 kBigStackSize = 512 * 1024; // 512 KiB
        static constexpr u16 kBigStackCount = 32;

        using StackIdQueue = moodycamel::ConcurrentQueue<u16>;
        StackIdQueue m_availableSmallStacksIds;
        StackIdQueue m_availableBigStacksIds;

        u8* m_smallStacks = nullptr;
        u8* m_bigStacks = nullptr;
    };
} // KryneEngine