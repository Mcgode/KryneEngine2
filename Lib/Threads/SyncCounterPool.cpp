/**
 * @file
 * @author Max Godefroy
 * @date 03/07/2022.
 */

#include <Common/Assert.hpp>
#include <Threads/FibersManager.hpp>
#include "SyncCounterPool.hpp"

namespace KryneEngine
{
    SyncCounterPool::SyncCounterPool()
    {
        moodycamel::ProducerToken producerToken(m_idQueue);

        for (u16 i = 0; i < (u16)kPoolSize; i++)
        {
            m_idQueue.enqueue(producerToken, i);
        }
    }

    SyncCounterId SyncCounterPool::AcquireCounter(u32 _initialValue)
    {
        const s32 initValue = static_cast<s32>(_initialValue);
        if (!Verify(initValue > 0))
        {
            return kInvalidSynCounterId;
        }

        u16 id;
        if (m_idQueue.try_dequeue(id))
        {
            m_entries[id].m_counter = initValue;
            return id;
        }
        return kInvalidSynCounterId;
    }

    void SyncCounterPool::AddWaitingJob(SyncCounterId _id, FiberJob *_newJob)
    {
        if (!Verify(_id >= 0 && _id < kPoolSize))
        {
            return;
        }

        auto& entry = m_entries[_id];
        const auto lock = entry.m_mutex.AutoLock();
        if (entry.m_counter == 0)
        {
            FibersManager::GetInstance()->QueueJob(_newJob);
        }
        else
        {
            m_entries[_id].m_waitingJobs.push_back(_newJob);
        }
    }

    u32 SyncCounterPool::DecrementCounterValue(SyncCounterId _id)
    {
        if (!Verify(_id >= 0 && _id < kPoolSize))
        {
            return 0;
        }

        auto& entry = m_entries[_id];

        const s32 value = --entry.m_counter;
        if (Verify(value >= 0))
        {
            if (value == 0)
            {
                // Use lock as
                const auto lock = entry.m_mutex.AutoLock();

                auto* fibersManager = FibersManager::GetInstance();
                for (auto* job: entry.m_waitingJobs)
                {
                    fibersManager->QueueJob(job);
                }
                entry.m_waitingJobs.clear();
            }

            return value;
        }

        return 0;
    }

    void SyncCounterPool::FreeCounter(SyncCounterId &_id)
    {
        if (!Verify(_id >= 0 && _id < kPoolSize))
        {
            return;
        }

        m_idQueue.enqueue(_id);
        _id = kInvalidSynCounterId;
    }
} // KryneEngine