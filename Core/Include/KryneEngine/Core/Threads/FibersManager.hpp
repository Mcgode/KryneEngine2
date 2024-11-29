/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include <EASTL/array.h>
#include <EASTL/unique_ptr.h>
#include <KryneEngine/Core/Threads/FiberJob.hpp>
#include <KryneEngine/Core/Threads/FiberThread.hpp>
#include <KryneEngine/Core/Threads/FiberTls.hpp>
#include <KryneEngine/Core/Threads/SyncCounterPool.hpp>

namespace KryneEngine
{
    struct FiberContextAllocator;
    class FiberThread;
    class IoQueryManager;

    class FibersManager
    {
        friend FiberThread;
        friend IoQueryManager;
        friend FiberContext;

    public:
        using Job = FiberJob*;

        explicit FibersManager(s32 _requestedThreadCount);

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

        void QueueJob(Job _job);

        void WaitForCounter(SyncCounterId _syncCounter);
        void ResetCounter(SyncCounterId _syncCounter);

        inline void WaitForCounterAndReset(SyncCounterId _syncCounter)
        {
            WaitForCounter(_syncCounter);
            ResetCounter(_syncCounter);
        }

        void YieldJob(Job _nextJob = nullptr);

        [[nodiscard]] IoQueryManager* GetIoQueryManager() const { return m_ioManager; }

    protected:

        bool _RetrieveNextJob(Job&job_, u16 _fiberIndex);

        void _OnContextSwitched();

        void _ThreadWaitForJob();

    private:
        using JobQueue = moodycamel::ConcurrentQueue<Job>;
        static constexpr u8 kJobQueuesCount = FiberJob::PriorityType::kJobPriorityTypes;
        eastl::array<JobQueue, kJobQueuesCount> m_jobQueues;

        using JobProducerTokenArray = eastl::array<moodycamel::ProducerToken, kJobQueuesCount>;
        FiberTls<JobProducerTokenArray> m_jobProducerTokens;

        using JobConsumerTokenArray = eastl::array<moodycamel::ConsumerToken, kJobQueuesCount>;
        FiberTls<JobConsumerTokenArray> m_jobConsumerTokens;

        DynamicArray<FiberThread> m_fiberThreads;

        FiberTls<Job> m_currentJobs;
        FiberTls<Job> m_nextJob;
        FiberTls<FiberContext> m_baseContexts;

        eastl::unique_ptr<FiberContextAllocator> m_contextAllocator;

        SyncCounterPool m_syncCounterPool {};

        std::mutex m_waitMutex;
        std::condition_variable m_waitVariable;

        static thread_local FibersManager* s_manager;
        IoQueryManager* m_ioManager = nullptr;
    };
} // KryneEngine