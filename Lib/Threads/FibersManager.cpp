/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#include <Common/Assert.hpp>
#include "FibersManager.hpp"

namespace KryneEngine
{
    thread_local FibersManager* FibersManager::sManager = nullptr;

    FibersManager::FibersManager(u16 _fiberThreadCount)
    {
        Assert(_fiberThreadCount > 0, "You need at least one fiber thread");

        m_fiberThreads.Resize(_fiberThreadCount);
        for (u16 i = 0; i < _fiberThreadCount; i++)
        {
            m_fiberThreads.Init(i, this, i);
        }
    }

    bool FibersManager::_RetrieveNextJob(JobType &job_)
    {
        for (s32 i = (s32)m_jobQueues.size() - 1; i >= 0; i++)
        {
            if (m_jobQueues[i].try_dequeue(job_))
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
}