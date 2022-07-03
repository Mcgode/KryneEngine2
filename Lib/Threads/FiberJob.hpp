/**
 * @file
 * @author Max Godefroy
 * @date 02/07/2022.
 */

#pragma once

#include <Common/KETypes.hpp>
#include <Common/Assert.hpp>
#include <Threads/Internal/UserContextSwitch.hpp>
#include <Threads/SyncCounterPool.hpp>

namespace KryneEngine
{
    class FiberJob
    {
    public:
        enum class Priority: u8
        {
            High,
            Medium,
            Low,
            Count
        };

        struct PriorityType
        {
            /// @details
            /// There are `Priority::Count` base type of priorities, and for each of them we distinguish between
            /// kicked jobs and unkicked ones. This allows to define the priority of one over the other.
            static constexpr u8 kJobPriorityTypes = 2 * static_cast<u8>(Priority::Count);

            bool m_pendingStart;
            Priority m_priority;

            PriorityType(Priority _priority, bool _pendingStart)
                : m_priority(_priority)
                , m_pendingStart(_pendingStart)
            {
                Assert(_priority != Priority::Count);
            }

            /// @details
            /// The lowest, the higher priority.
            /// Since we want to finish the kicked jobs before starting new ones, but still want to have the higher
            /// priority jobs before the lower ones, we also use priority to apply order.
            ///
            /// Resulting table is:
            /// - 0: High, kicked
            /// - 1: High, not kicked
            /// - 2: Medium, kicked
            /// - 3: Medium, not kicked
            /// - 4: Low, kicked
            /// - 5: Low, not kicked
            explicit operator u8() const
            {
                return
                        static_cast<u8>(m_pendingStart) |
                        (static_cast<u8>(m_priority) << 1);
            }
        };

        enum class Status
        {
            PendingStart,
            Running,
            Paused,
            Finished
        };

        friend class FibersManager;
        friend class FiberThread;

    public:
        typedef void (JobFunc)(void*);

        explicit FiberJob(JobFunc* _func, void* _userData, Priority _priority = Priority::Medium,
                          bool _bigStack = false);

        [[nodiscard]] Status GetStatus() const { return m_status; }

        [[nodiscard]] PriorityType GetPriorityType() const
        {
            return { m_priority, m_status == Status::PendingStart };
        }

        [[nodiscard]] bool CanRun() const
        {
            const Status status = m_status;
            return status == Status::PendingStart || status == Status::Paused;
        }

        [[nodiscard]] u32 GetWaitingForCounterValue() const { return m_waitingForCounterValue; }

    protected:
        [[nodiscard]] bool _HasStackAssigned() const { return m_stackId != kInvalidStackId; }

        void _SetStackPointer(u16 _stackId, u8 *_stackPtr, u64 _stackSize);

        void _ResetStackPointer();

    private:
        JobFunc* m_functionPtr;
        void* m_userData;
        const Priority m_priority;
        const bool m_bigStack;

        volatile Status m_status = Status::PendingStart;

        static constexpr s32 kInvalidStackId = -1;
        s32 m_stackId = kInvalidStackId;
        FiberContext m_context {};

        SyncCounterId m_associatedCounterId = kInvalidSynCounterId;
        u32 m_waitingForCounterValue = 0;

        static void _KickJob();
    };
} // KryneEngine