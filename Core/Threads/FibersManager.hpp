/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include <EASTL/array.h>
#include <Threads/FiberThread.hpp>
#include <Threads/FiberJob.hpp>
#include <Threads/FiberTls.hpp>
#include <Threads/SyncCounterPool.hpp>

namespace KryneEngine
{
    class IoQueryManager;

    class FibersManager
    {
        friend FiberThread;
        friend FiberContext;
        friend IoQueryManager;

    public:
        using JobType = FiberJob*;

        explicit FibersManager(u16 _fiberThreadCount);

        ~FibersManager();

        [[nodiscard]] static FibersManager* GetInstance();

        [[nodiscard]] static u16 GetFibersCount()
        {
            const auto* manager = GetInstance();
            if (KE_VERIFY(manager != nullptr))
            {
                return manager->GetFiberThreadCount();
            }
        	return 0;
        }

        [[nodiscard]] u16 GetFiberThreadCount() const { return m_fiberThreads.Size(); }

        [[nodiscard]] FiberJob* GetCurrentJob();

        [[nodiscard]] SyncCounterId InitAndBatchJobs(FiberJob* _jobArray,
                                                     FiberJob::JobFunc* _jobFunc,
                                                     void* _userData,
                                                     u32 _count = 1,
                                                     FiberJob::Priority _priority = FiberJob::Priority::Medium,
                                                     bool _useBigStack = false);

        [[nodiscard]] SyncCounterPool::AutoSyncCounter AcquireAutoSyncCounter(u32 _count = 1);

        void QueueJob(JobType _job);

        void WaitForCounter(SyncCounterId _syncCounter);
        void ResetCounter(SyncCounterId _syncCounter);

        inline void WaitForCounterAndReset(SyncCounterId _syncCounter)
        {
            WaitForCounter(_syncCounter);
            ResetCounter(_syncCounter);
        }

        void YieldJob(JobType _nextJob = nullptr);

        [[nodiscard]] IoQueryManager* GetIoQueryManager() const { return m_ioManager; }

    protected:

        bool _RetrieveNextJob(JobType &job_, u16 _fiberIndex);

        void _OnContextSwitched();

        void _ThreadWaitForJob();

    private:
        using JobQueue = moodycamel::ConcurrentQueue<JobType>;
        static constexpr u8 kJobQueuesCount = FiberJob::PriorityType::kJobPriorityTypes;
        eastl::array<JobQueue, kJobQueuesCount> m_jobQueues;

        using JobProducerTokenArray = eastl::array<moodycamel::ProducerToken, kJobQueuesCount>;
        FiberTls<JobProducerTokenArray> m_jobProducerTokens;

        using JobConsumerTokenArray = eastl::array<moodycamel::ConsumerToken, kJobQueuesCount>;
        FiberTls<JobConsumerTokenArray> m_jobConsumerTokens;

        DynamicArray<FiberThread> m_fiberThreads;

        FiberTls<JobType> m_currentJobs;
        FiberTls<JobType> m_nextJob;
        FiberTls<FiberContext> m_baseContexts;

        FiberContextAllocator m_contextAllocator;

        SyncCounterPool m_syncCounterPool {};

        std::mutex m_waitMutex;
        std::condition_variable m_waitVariable;

        static thread_local FibersManager* s_manager;
        IoQueryManager* m_ioManager = nullptr;
    };
} // KryneEngine