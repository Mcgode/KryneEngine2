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

    FibersManager::JobType FibersManager::_RetrieveNextJob()
    {
        for (s32 i = (s32)m_jobQueues.size() - 1; i >= 0; i++)
        {
        }
        return nullptr;
    }

    FibersManager *FibersManager::GetInstance()
    {
        return sManager;
    }
}