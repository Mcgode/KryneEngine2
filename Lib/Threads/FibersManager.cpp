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
            m_jobProducerTokens.Init(this, [this](JobProducerTokenArray &_array)
            {
                for (u64 i = 0; i < _array.size(); i++)
                {
                    // Do in-place memory init, else it will try to interpret uninitialized memory as a valid object.
                    ::new(&_array[i]) moodycamel::ProducerToken(m_jobQueues[i]);
                }
            });

            m_jobConsumerTokens.Init(this, [this](JobConsumerTokenArray &_array)
            {
                for (u64 i = 0; i < _array.size(); i++)
                {
                    // Do in-place memory init, else it will try to interpret uninitialized memory as a valid object.
                    ::new(&_array[i]) moodycamel::ConsumerToken(m_jobQueues[i]);
                }
            });
        }

        for (u16 i = 0; i < _fiberThreadCount; i++)
        {
            m_fiberThreads.Init(i, this, i);
        }
    }

    bool FibersManager::_RetrieveNextJob(JobType &job_, u16 _fiberIndex)
    {
        auto& consumerTokens = m_jobConsumerTokens.Load(_fiberIndex);
        for (s32 i = (s32)m_jobQueues.size() - 1; i >= 0; i--)
        {
            if (m_jobQueues[i].try_dequeue(consumerTokens[i], job_))
            {
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
    }
}