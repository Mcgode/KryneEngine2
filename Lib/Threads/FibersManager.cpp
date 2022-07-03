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
    thread_local FibersManager* FibersManager::sManager = nullptr;

    FibersManager::FibersManager(u16 _fiberThreadCount)
    {
        Assert(_fiberThreadCount > 0, "You need at least one fiber thread");

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
            m_contexts.Init(this, {});
        }

        {
            for (u16 i = 0; i < kSmallStackCount; i++)
            {
                m_availableSmallStacksIds.enqueue(i);
            }
            m_smallStacks = new u8[kSmallStackSize * (u64) kSmallStackCount];

            for (u16 i = 0; i < kBigStackCount; i++)
            {
                m_availableBigStacksIds.enqueue(i);
            }
            m_bigStacks = new u8[kBigStackSize * (u64) kBigStackCount];
        }

        for (u16 i = 0; i < _fiberThreadCount; i++)
        {
            m_fiberThreads.Init(i, this, i);
        }
    }

    bool FibersManager::_RetrieveNextJob(JobType &job_, u16 _fiberIndex)
    {
        auto& consumerTokens = m_jobConsumerTokens.Load(_fiberIndex);
        for (s64 i = 0; i < (s64)kJobQueuesCount; i++)
        {
            if (m_jobQueues[i].try_dequeue(consumerTokens[i], job_))
            {
                if (!job_->_HasStackAssigned())
                {
                    Assert(job_->GetStatus() == FiberJob::Status::PendingStart);

                    const bool useBigStack = job_->m_bigStack;
                    auto& queue = useBigStack ? m_availableBigStacksIds : m_availableSmallStacksIds;

                    u16 stackId;
                    if (!Verify(queue.try_dequeue(stackId), "Out of Fiber stacks!"))
                    {
                        m_jobQueues[i].enqueue(m_jobProducerTokens.Load(_fiberIndex)[i], job_);
                        return false;
                    }

                    auto* bufferPtr = useBigStack
                            ? &m_bigStacks[kBigStackSize * stackId]
                            : &m_smallStacks[kSmallStackSize * stackId];
                    job_->_SetStackPointer(stackId, bufferPtr, useBigStack ? kBigStackSize : kSmallStackSize);
                }
                else if (job_->GetStatus() == FiberJob::Status::Finished || job_->GetStatus() == FiberJob::Status::Running)
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
        return sManager;
    }

    FibersManager::~FibersManager()
    {
        // Make sure to end and join all the fiber threads before anything else.
        m_fiberThreads.Clear();

        delete m_smallStacks;
        delete m_bigStacks;
    }

    FiberJob *FibersManager::GetCurrentJob()
    {
        return m_currentJobs.Load();
    }

    void FibersManager::YieldJob()
    {
        const auto fiberIndex = FiberThread::GetCurrentFiberThreadIndex();
        auto* currentJob = m_currentJobs.Load(fiberIndex);

        if (currentJob != nullptr && currentJob->GetStatus() == FiberJob::Status::Running)
        {
            currentJob->m_status = FiberJob::Status::Paused;
        }

        m_fiberThreads[fiberIndex].SwitchToNextJob(this, currentJob);
    }

    void FibersManager::_OnContextSwitched()
    {
        const auto fiberIndex = FiberThread::GetCurrentFiberThreadIndex();

        auto& oldJob = m_currentJobs.Load(fiberIndex);
        auto& newJob = m_nextJob.Load(fiberIndex);

        if (oldJob != nullptr && oldJob->GetStatus() == FiberJob::Status::Finished)
        {
            // Free stack pointer
            if (oldJob->m_bigStack)
            {
                m_availableBigStacksIds.enqueue(oldJob->m_stackId);
            }
            else
            {
                m_availableSmallStacksIds.enqueue(oldJob->m_stackId);
            }

            oldJob->_ResetStackPointer();
        }

        oldJob = newJob;
        newJob = nullptr;
    }
}