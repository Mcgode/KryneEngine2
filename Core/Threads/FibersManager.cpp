/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#include "FibersManager.hpp"
#include <Common/Assert.hpp>
#include <Threads/FiberTls.inl>

namespace KryneEngine
{
    thread_local FibersManager* FibersManager::s_manager = nullptr;

    FibersManager::FibersManager(u16 _fiberThreadCount)
    {
        KE_ASSERT_MSG(_fiberThreadCount > 0, "You need at least one fiber thread");

        // Resize array first!
        // This size is used to init the FiberTls objects,
        m_fiberThreads.Resize(_fiberThreadCount);

        // Init FiberTls objects before initializing the threads, to avoid racing conditions.
        {
            m_jobProducerTokens.InitFunc(this, [this](JobProducerTokenArray &_array)
            {
                for (u64 i = 0; i < _array.size(); i++)
                {
                    // Do in-place memory init, else it will try to interpret uninitialized memory as a valid object.
                    ::new(&_array[i]) moodycamel::ProducerToken(m_jobQueues[i]);
                }
            });

            m_jobConsumerTokens.InitFunc(this, [this](JobConsumerTokenArray &_array)
            {
                for (u64 i = 0; i < _array.size(); i++)
                {
                    // Do in-place memory init, else it will try to interpret uninitialized memory as a valid object.
                    ::new(&_array[i]) moodycamel::ConsumerToken(m_jobQueues[i]);
                }
            });

            m_currentJobs.Init(this, nullptr);
            m_nextJob.Init(this, nullptr);
            m_baseContexts.Init(this, {});
        }

        for (u16 i = 0; i < _fiberThreadCount; i++)
        {
            m_fiberThreads.Init(i, this, i);
        }
    }

    void FibersManager::QueueJob(FibersManager::JobType _job)
    {
        VERIFY_OR_RETURN_VOID(_job != nullptr && _job->m_associatedCounterId != kInvalidSyncCounterId);

        KE_ASSERT(_job->CanRun());

        const u8 priorityId = (u8)_job->GetPriorityType();
        if (FiberThread::IsFiberThread())
        {
            auto& producerToken = m_jobProducerTokens.Load()[priorityId];
            m_jobQueues[priorityId].enqueue(producerToken, _job);
        }
        else
        {
            m_jobQueues[priorityId].enqueue(_job);
        }
        m_waitVariable.notify_one();
    }

    bool FibersManager::_RetrieveNextJob(JobType &job_, u16 _fiberIndex)
    {
        auto& consumerTokens = m_jobConsumerTokens.Load(_fiberIndex);
        for (s64 i = 0; i < (s64)kJobQueuesCount; i++)
        {
            if (m_jobQueues[i].try_dequeue(consumerTokens[i], job_))
            {
                if (!job_->_HasContextAssigned())
                {
                    KE_ASSERT(job_->GetStatus() == FiberJob::Status::PendingStart);

                    u16 id;
                    if (m_contextAllocator.Allocate(job_->m_bigStack, id))
                    {
                        job_->_SetContext(id, m_contextAllocator.GetContext(id));
                    }
                }
                else if (!job_->CanRun())
                {
                    // If job is already finished or still running, ignore it and keep trying to retrieve the next job.
                    // This might happen because the job was run by skipping this step, which is legal.
                    job_ = nullptr;
                    i--; // Roll back index to try retrieving again from this queue.
                    continue;
                }
                return true;
            }
        }
        return false;
    }

    FibersManager *FibersManager::GetInstance()
    {
        return s_manager;
    }

    FibersManager::~FibersManager()
    {
        for (auto& fiberThread: m_fiberThreads)
        {
            fiberThread.Stop(m_waitVariable);
        }
        // Make sure to end and join all the fiber threads before anything else.
        m_fiberThreads.Clear();
    }

    FiberJob *FibersManager::GetCurrentJob()
    {
        return m_currentJobs.Load();
    }

    void FibersManager::YieldJob(JobType _nextJob)
    {
        const auto fiberIndex = FiberThread::GetCurrentFiberThreadIndex();
        auto* currentJob = m_currentJobs.Load(fiberIndex);

        if (currentJob != nullptr && currentJob->GetStatus() == FiberJob::Status::Running)
        {
            currentJob->m_status = FiberJob::Status::Paused;
            QueueJob(currentJob);
        }

        IF_NOT_VERIFY(_nextJob == nullptr || _nextJob->CanRun())
        {
            _nextJob = nullptr;
        }

        m_fiberThreads[fiberIndex].SwitchToNextJob(this, currentJob, _nextJob);
    }

    void FibersManager::_OnContextSwitched()
    {
        const auto fiberIndex = FiberThread::GetCurrentFiberThreadIndex();

        auto& oldJob = m_currentJobs.Load(fiberIndex);
        auto& newJob = m_nextJob.Load(fiberIndex);

        if (oldJob != nullptr && oldJob->GetStatus() == FiberJob::Status::Finished)
        {
            // Decrement counter
            m_syncCounterPool.DecrementCounterValue(oldJob->m_associatedCounterId);

            m_contextAllocator.Free(oldJob->m_contextId);
            oldJob->_ResetContext();
        }

        oldJob = newJob;
        newJob = nullptr;
    }

    SyncCounterId FibersManager::InitAndBatchJobs(FiberJob *_jobArray,
                                                  FiberJob::JobFunc *_jobFunc,
                                                  void *_userData,
                                                  u32 _count,
                                                  FiberJob::Priority _priority,
                                                  bool _useBigStack)
    {
        const auto syncCounter = m_syncCounterPool.AcquireCounter(_count);

        VERIFY_OR_RETURN(syncCounter != kInvalidSyncCounterId, kInvalidSyncCounterId);

        for (u32 i = 0; i < _count; i++)
        {
            auto& job = _jobArray[i];
            job.m_functionPtr = _jobFunc;
            job.m_userData = _userData;
            job.m_priority = _priority;
            job.m_bigStack = _useBigStack;
            job.m_associatedCounterId = syncCounter;
            QueueJob(&job);
        }

        return syncCounter;
    }

    SyncCounterPool::AutoSyncCounter FibersManager::AcquireAutoSyncCounter(u32 _count)
    {
        return eastl::move(m_syncCounterPool.AcquireAutoCounter(_count));
    }

    void FibersManager::WaitForCounter(SyncCounterId _syncCounter)
    {
        auto* currentJob = GetCurrentJob();
        m_syncCounterPool.AddWaitingJob(_syncCounter, currentJob);

        // Manually pause here, to avoid auto re-queueing when yielding.
        currentJob->m_status = FiberJob::Status::Paused;

        YieldJob();
    }

    void FibersManager::ResetCounter(SyncCounterId _syncCounter)
    {
        m_syncCounterPool.FreeCounter(_syncCounter);
    }

    void FibersManager::_ThreadWaitForJob()
    {
        std::unique_lock<std::mutex> lock(m_waitMutex);
        m_waitVariable.wait(lock); // Allow spurious wakeup.
    }
}