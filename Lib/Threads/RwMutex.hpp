/**
 * @file
 * @author Max Godefroy
 * @date 16/11/2022.
 */

#pragma once

#include <Threads/Semaphore.hpp>
#include <Common/BitUtils.hpp>

namespace KryneEngine
{
    /**
     * @details
     * Based on https://github.com/preshing/cpp11-on-multicore/blob/master/common/rwlock.h
     */
    template<class Semaphore>
    class RwMutexBase
    {
    public:
        explicit RwMutexBase()
            : m_status(0)
            , m_readerSemaphore(0)
            , m_writerSemaphore(0)
        {}

        void LockReader() noexcept
        {
            Status oldStatus = m_status.load(std::memory_order::relaxed);
            Status newStatus;

            do
            {
                newStatus = oldStatus;

                if (oldStatus.m_writers > 0)
                {
                    newStatus.m_waitingToRead++;
                }
                else
                {
                    newStatus.m_readers++;
                }
            }
            while(!m_status.compare_exchange_weak(
                    oldStatus,
                    newStatus,
                    std::memory_order::acquire,
                    std::memory_order::relaxed));

            if (oldStatus.m_writers > 0)
            {
                // Writers have priority (to avoid starving).
                // Readers will wait until writing is done.
                m_readerSemaphore.Wait();
            }
        }

        void UnlockReader() noexcept
        {
            Status oldStatus = m_status.fetch_sub(Status().m_readers.One(), std::memory_order::release);
            Assert(!oldStatus.m_readers.IsZero());
            if (oldStatus.m_readers == 1 && !oldStatus.m_writers.IsZero())
            {
                m_writerSemaphore.SignalOnce();
            }
        }

        void LockWriter() noexcept
        {
            Status oldStatus = m_status.fetch_add(Status().m_writers.One(), std::memory_order_release);
            Assert(oldStatus.m_writers + 1 <= Status().m_writers.Maximum());
            if (!oldStatus.m_readers.IsZero() || !oldStatus.m_writers.IsZero())
            {
                m_writerSemaphore.Wait();
            }
        }

        void UnlockWriter() noexcept
        {
            Status oldStatus = m_status.load(std::memory_order::relaxed);
            Status newStatus;
            u32 waitingToRead = 0;
            do
            {
                Assert(oldStatus.m_readers.IsZero());

                newStatus = oldStatus;
                newStatus.m_writers--;

                // Process the reader requests accumulated during writer mode.
                // Avoids reader starving.
                waitingToRead = oldStatus.m_waitingToRead;
                if (waitingToRead > 0)
                {
                    newStatus.m_waitingToRead = 0;
                    newStatus.m_readers = waitingToRead;
                }
            }
            while (!m_status.compare_exchange_weak(
                    oldStatus,
                    newStatus,
                    std::memory_order::release,
                    std::memory_order::relaxed));

            if (waitingToRead > 0)
            {
                m_readerSemaphore.Signal(waitingToRead);
            }
            else if (!newStatus.m_writers.IsZero())
            {
                m_writerSemaphore.SignalOnce();
            }
        }

    protected:
        Semaphore m_readerSemaphore;
        Semaphore m_writerSemaphore;

    private:
        union Status
        {
            u32 m_rawValue = 0;
            BitUtils::BitFieldMember<u32, 11, 0> m_readers;
            BitUtils::BitFieldMember<u32, 11, 11> m_waitingToRead;
            BitUtils::BitFieldMember<u32, 10, 22> m_writers;

            Status(u32 _v = 0) { m_rawValue = _v; }
            Status& operator =(u32 _v) { m_rawValue = _v; return *this; }
            operator u32() const { return m_rawValue; }
            operator u32&() { return m_rawValue; }
        };

        std::atomic<u32> m_status {};

        using ReaderLockGuardT = Threads::SyncLockGuard<RwMutexBase, &RwMutexBase::LockReader, &RwMutexBase::UnlockReader>;
        using WriterLockGuardT = Threads::SyncLockGuard<RwMutexBase, &RwMutexBase::LockWriter, &RwMutexBase::UnlockWriter>;

    public:
        ReaderLockGuardT AutoLockReader()
        {
            return ReaderLockGuardT(this);
        }

        WriterLockGuardT AutoLockWriter()
        {
            return WriterLockGuardT(this);
        }
    };

    /**
     * @brief A RW mutex safe to use in fibers.
     */
     class BusySpinRwMutex: RwMutexBase<BusySpinSemaphore>
     {};
}
