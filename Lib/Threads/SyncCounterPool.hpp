/**
 * @file
 * @author Max Godefroy
 * @date 03/07/2022.
 */

#pragma once

#include <atomic>
#include <Common/KETypes.hpp>
#include <EASTL/fixed_vector.h>
#include <EASTL/array.h>
#include <moodycamel/concurrentqueue.h>
#include "LightweightMutex.hpp"

namespace KryneEngine
{
    using SyncCounterId = s32;
    static constexpr SyncCounterId kInvalidSynCounterId = -1;

    class FiberJob;

    class SyncCounterPool
    {
    public:
        SyncCounterPool();

        SyncCounterId AcquireCounter(u32 _initialValue);

        void AddWaitingJob(SyncCounterId _id, FiberJob* _newJob);

        u32 DecrementCounterValue(SyncCounterId _id);

        void FreeCounter(SyncCounterId &_id);

    private:
        struct Entry
        {
            std::atomic<s32> m_counter;
            eastl::fixed_vector<FiberJob*, 4> m_waitingJobs;
            LightweightMutex m_mutex;
        };

        static constexpr u16 kPoolSize = 128;
        eastl::array<Entry, kPoolSize> m_entries;

        moodycamel::ConcurrentQueue<SyncCounterId> m_idQueue;
    };
} // KryneEngine