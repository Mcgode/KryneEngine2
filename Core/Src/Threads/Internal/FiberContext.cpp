/**
 * @file
 * @author Max Godefroy
 * @date 02/07/2022.
 */

#include "FiberContext.hpp"

#include <EASTL/allocator.h>

#include "KryneEngine/Core/Common/Assert.hpp"
#include "KryneEngine/Core/Threads/FibersManager.hpp"
#include "KryneEngine/Core/Profiling/TracyHeader.hpp"


namespace KryneEngine
{
    void FiberContext::SwapContext(FiberContext *_new)
    {
        TracyFiberLeave;

        const boost::context::detail::transfer_t t = boost::context::detail::jump_fcontext(
            _new->m_context,
            this);

        if (KE_VERIFY(t.data != nullptr))
        {
            static_cast<FiberContext*>(t.data)->m_context = t.fctx;
        }

        TracyFiberEnter(m_name.c_str());
    }

    void FiberContext::RunFiber(boost::context::detail::transfer_t _transfer)
    {
        const auto fibersManager = FibersManager::GetInstance();
        VERIFY_OR_RETURN_VOID(fibersManager != nullptr);
        fibersManager->_OnContextSwitched();

        if (KE_VERIFY(_transfer.data != nullptr))
        {
            static_cast<FiberContext*>(_transfer.data)->m_context = _transfer.fctx;
        }

        while (true)
        {
            auto* job = fibersManager->GetCurrentJob();

            TracyFiberEnter(job->m_context->m_name.c_str());

            if (KE_VERIFY(job->m_status == FiberJob::Status::PendingStart))
            {
                job->m_status = FiberJob::Status::Running;
                job->m_functionPtr(job->m_userData);
                job->m_status = FiberJob::Status::Finished;
            }

            fibersManager->YieldJob();
        }
    }

    FiberContextAllocator::FiberContextAllocator(AllocatorInstance _allocator)
    {
        m_availableSmallContextsIds.m_priorityQueue.get_container().set_allocator(_allocator);
        m_availableBigContextsIds.m_priorityQueue.get_container().set_allocator(_allocator);

        {
            const auto smallLock = m_availableSmallContextsIds.m_spinLock.AutoLock();
            const auto bigLock = m_availableBigContextsIds.m_spinLock.AutoLock();

            m_availableSmallContextsIds.m_priorityQueue.get_container().reserve(kSmallStackCount);
            for (u16 i = 0; i < kSmallStackCount; i++)
            {
                m_availableSmallContextsIds.m_priorityQueue.push(i);
            }

            m_availableBigContextsIds.m_priorityQueue.get_container().reserve(kBigStackCount);
            for (u16 i = 0; i < kBigStackCount; i++)
            {
                m_availableBigContextsIds.m_priorityQueue.push(i + kSmallStackCount);
            }

            m_smallStacks = static_cast<SmallStack*>(_allocator.allocate(
                sizeof(SmallStack) * static_cast<size_t>(kSmallStackCount),
                kStackAlignment));
            m_bigStacks = static_cast<BigStack*>(_allocator.allocate(
                sizeof(BigStack) * static_cast<size_t>(kBigStackCount),
                kStackAlignment));

            for (u32 i = 0; i < m_contexts.size(); i++)
            {
                auto& context = m_contexts[i];

                if (i < kSmallStackCount)
                {
                    context.m_context = make_fcontext(
                        m_smallStacks + i + 1, // Do a +1, because stack begins from the end
                        sizeof(SmallStack),
                        FiberContext::RunFiber);
                    context.m_name.sprintf("Fiber %d", i);
                }
                else
                {
                    const u16 index = i - kSmallStackCount;
                    context.m_context = make_fcontext(
                        m_bigStacks + index + 1, // Do a +1, because stack begins from the end
                        sizeof(BigStack),
                        FiberContext::RunFiber);
                    context.m_name.sprintf("Big Fiber %d", index);
                }
            }
        }
    }

    FiberContextAllocator::~FiberContextAllocator()
    {
        AllocatorInstance allocator {};
        allocator.deallocate(m_smallStacks);
        allocator.deallocate(m_bigStacks);
    }

    bool FiberContextAllocator::Allocate(bool _bigStack, u16 &id_)
    {
        auto& queue = _bigStack
                ? m_availableBigContextsIds
                : m_availableSmallContextsIds;

        const auto lock = queue.m_spinLock.AutoLock();
        IF_NOT_VERIFY_MSG(!queue.m_priorityQueue.empty(), "Out of Fiber stacks!")
        {
            return false;
        }
        queue.m_priorityQueue.pop(id_);
        return true;
    }

    void FiberContextAllocator::Free(u16 _id)
    {
        VERIFY_OR_RETURN_VOID(_id < m_contexts.size());

        if (_id < kSmallStackCount)
        {
            const auto lock = m_availableSmallContextsIds.m_spinLock.AutoLock();
            m_availableSmallContextsIds.m_priorityQueue.push(_id);
        }
        else
        {
            const auto lock = m_availableBigContextsIds.m_spinLock.AutoLock();
            m_availableBigContextsIds.m_priorityQueue.push(_id);
        }
    }

    FiberContext *FiberContextAllocator::GetContext(u16 _id)
    {
        VERIFY_OR_RETURN(_id < m_contexts.size(), nullptr);
        return &m_contexts[_id];
    }
}
