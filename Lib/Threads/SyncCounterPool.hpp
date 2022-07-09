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
    struct SyncCounterId
    {
        friend class SyncCounterPool;

    public:
        SyncCounterId() = default;

        inline bool operator ==(const SyncCounterId& _other) const { return m_id == _other.m_id; }

    private:
        s32 m_id = -1;

        SyncCounterId(s32 _value): m_id(_value) {}

        operator s32() const { return m_id; }
    };
    static constexpr SyncCounterId kInvalidSynCounterId = SyncCounterId();

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

        moodycamel::ConcurrentQueue<u16> m_idQueue;
    };
} // KryneEngine