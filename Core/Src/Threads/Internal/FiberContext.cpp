/**
 * @file
 * @author Max Godefroy
 * @date 02/07/2022.
 */

#include "FiberContext.hpp"

#if CONTEXT_SWITCH_WINDOWS_FIBERS
#	include <Platform/Windows.h>
#endif

#include "KryneEngine/Core/Common/Assert.hpp"
#include "KryneEngine/Core/Threads/FibersManager.hpp"
#include "KryneEngine/Core/Profiling/TracyHeader.hpp"


namespace KryneEngine
{
    void FiberContext::SwapContext(FiberContext *_new)
    {
        TracyFiberLeave;
#if CONTEXT_SWITCH_WINDOWS_FIBERS
        KE_ASSERT(m_winFiber == GetCurrentFiber());

        SwitchToFiber((LPVOID*)_new->m_winFiber);
#else
        //swapcontext(&m_uContext, &_new->m_uContext);
#endif

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

    FiberContextAllocator::FiberContextAllocator()
    {
        {

            const auto smallLock = m_availableSmallContextsIds.m_spinLock.AutoLock();
            const auto bigLock = m_availableBigContextsIds.m_spinLock.AutoLock();

            for (u16 i = 0; i < kSmallStackCount; i++)
            {
                m_availableSmallContextsIds.m_priorityQueue.push(i);
            }

            for (u16 i = 0; i < kBigStackCount; i++)
            {
                m_availableBigContextsIds.m_priorityQueue.push(i + kSmallStackCount);
            }

            m_smallStacks = static_cast<SmallStack*>(std::aligned_alloc(
                kStackAlignment,
                sizeof(SmallStack) * static_cast<size_t>(kSmallStackCount)));
            m_bigStacks = static_cast<BigStack*>(std::aligned_alloc(
                kStackAlignment,
                sizeof(BigStack) * static_cast<size_t>(kBigStackCount)));

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

#if CONTEXT_SWITCH_WINDOWS_FIBERS

            for (u32 i = 0; i < m_contexts.size(); i++)
            {
                auto& context = m_contexts[i];
                if (i < kSmallStackCount)
                {
                    context.m_winFiber = CreateFiber(kSmallStackSize, FiberContext::RunFiber, nullptr);
                    KE_ASSERT_FATAL(context.m_winFiber != nullptr);
                    context.m_name.sprintf("Fiber %d", i);
                }
                else
                {
                    context.m_winFiber = CreateFiber(kBigStackSize, FiberContext::RunFiber, nullptr);
                    KE_ASSERT_FATAL(context.m_winFiber != nullptr);
                    context.m_name.sprintf("Big Fiber %d", i - kSmallStackCount);
                }
            }

#elif CONTEXT_SWITCH_UCONTEXT && 0

            m_smallStacks = static_cast<u8*>(std::aligned_alloc(
                kStackAlignment,
                kSmallStackSize * static_cast<size_t>(kSmallStackCount)));
            m_bigStacks = static_cast<u8*>(std::aligned_alloc(
                kStackAlignment,
                kBigStackSize * static_cast<size_t>(kBigStackCount)));
            m_bigStacks = new u8[kBigStackSize * static_cast<u64>(kBigStackCount)];

            for (u32 i = 0; i < m_contexts.size(); i++)
            {
                auto& context = m_contexts[i];
                getcontext(&context.m_uContext);

                if (i < kSmallStackCount)
                {
                    context.m_uContext.uc_stack.ss_sp = m_smallStacks + i * kSmallStackSize;
                    context.m_uContext.uc_stack.ss_size = kSmallStackSize;
                    context.m_name.sprintf("Fiber %d", i);
                }
                else
                {
                    const u16 bigStackIndex = i - kSmallStackCount;
                    context.m_uContext.uc_stack.ss_sp = m_bigStacks + bigStackIndex * kBigStackSize;
                    context.m_uContext.uc_stack.ss_size = kBigStackSize;
                    context.m_name.sprintf("Big Fiber %d", bigStackIndex);
                }
                context.m_uContext.uc_link = nullptr;
                makecontext(&context.m_uContext, FiberContext::RunFiber, 0);
            }

#elif CONTEXT_SWITCH_ABI_WINDOWS || CONTEXT_SWITCH_ABI_SYS_V

            m_smallStacks = new u8[kSmallStackSize * (u64) kSmallStackCount];
            m_bigStacks = new u8[kBigStackSize * (u64) kBigStackCount];

#endif
        }
    }

    FiberContextAllocator::~FiberContextAllocator()
    {
        for (FiberContext& context: m_contexts)
        {
#if CONTEXT_SWITCH_WINDOWS_FIBERS
            DeleteFiber(context.m_winFiber);
#endif
        }

#if CONTEXT_SWITCH_UCONTEXT
        std::free(m_smallStacks);
        std::free(m_bigStacks);
#endif
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
