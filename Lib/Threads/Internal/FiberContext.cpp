/**
 * @file
 * @author Max Godefroy
 * @date 02/07/2022.
 */

#include <Common/Assert.hpp>
#include <Threads/FibersManager.hpp>
#include "FiberContext.hpp"

namespace
{
    // Assembly code pulled from https://graphitemaster.github.io/fibers/#user-space-context-switching
#if defined(_MSC_VER) && !defined(__clang__)
    #pragma section(".text")
    __declspec(allocate(".text"))
#else
    __attribute__((section(".text#")))
#endif
    const unsigned char swap_context_sysV_asm[] = {
            0x4c, 0x8b, 0x04, 0x24, // mov (%rsp), %r8
            0x4c, 0x89, 0x07, // mov %r8, (%rdi)
            0x4c, 0x8d, 0x44, 0x24, 0x08, // lea 0x8(%rsp), %r8
            0x4c, 0x89, 0x47, 0x08, // mov %r8, 0x8(%rdi)
            0x48, 0x89, 0x5f, 0x10, // mov %rbx, 0x10(%rdi)
            0x48, 0x89, 0x6f, 0x18, // mov %rbp, 0x18(%rdi)
            0x4c, 0x89, 0x67, 0x20, // mov %r12, 0x20(%rdi)
            0x4c, 0x89, 0x6f, 0x28, // mov %r13, 0x28(%rdi)
            0x4c, 0x89, 0x77, 0x30, // mov %r14, 0x30(%rdi)
            0x4c, 0x89, 0x7f, 0x38, // mov %r15, 0x38(%rdi)
            0x4c, 0x8b, 0x06, // mov (%rsi), %r8
            0x48, 0x8b, 0x66, 0x08, // mov 0x8(%rsi), %rsp
            0x48, 0x8b, 0x5e, 0x10, // mov 0x10(%rsi), %rbx
            0x48, 0x8b, 0x6e, 0x18, // mov 0x18(%rsi), %rbp
            0x4c, 0x8b, 0x66, 0x20, // mov 0x20(%rsi), %r12
            0x4c, 0x8b, 0x6e, 0x28, // mov 0x28(%rsi), %r13
            0x4c, 0x8b, 0x76, 0x30, // mov 0x30(%rsi), %r14
            0x4c, 0x8b, 0x7e, 0x38, // mov 0x38(%rsi), %r15
            0x41, 0x50, // push %r8
            0x31, 0xc0, // xor %eax, %eax
            0xc3 // retq
    };

#if defined(_MSC_VER)
    #pragma section(".text")
    __declspec(allocate(".text"))
#else
    __attribute__((section(".text#")))
#endif
    const unsigned char swap_context_win_asm[] = {
            0x4c, 0x8b, 0x04, 0x24, // mov (%rsp),%r8
            0x4c, 0x89, 0x02, // mov %r8,(%rdx)
            0x4c, 0x8d, 0x44, 0x24, 0x08, // lea 0x8(%rsp),%r8
            0x4c, 0x89, 0x42, 0x08, // mov %r8,0x8(%rdx)
            0x48, 0x89, 0x5a, 0x10, // mov %rbx,0x10(%rdx)
            0x48, 0x89, 0x6a, 0x18, // mov %rbp,0x18(%rdx)
            0x4c, 0x89, 0x62, 0x20, // mov %r12,0x20(%rdx)
            0x4c, 0x89, 0x6a, 0x28, // mov %r13,0x28(%rdx)
            0x4c, 0x89, 0x72, 0x30, // mov %r14,0x30(%rdx)
            0x4c, 0x89, 0x7a, 0x38, // mov %r15,0x38(%rdx)
            0x48, 0x89, 0x7a, 0x40, // mov %rdi,0x40(%rdx)
            0x48, 0x89, 0x72, 0x48, // mov %rsi,0x48(%rdx)
            0x0f, 0x11, 0x72, 0x50, // movups %xmm6,0x50(%rdx)
            0x0f, 0x11, 0x7a, 0x60, // movups %xmm7,0x60(%rdx)
            0x44, 0x0f, 0x11, 0x42, 0x70, // movups %xmm8,0x70(%rdx)
            0x44, 0x0f, 0x11, 0x8a, 0x80, // movups %xmm9,0x80(%rdx)
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x11, 0x92, 0x90, // movups %xmm10,0x90(%rdx)
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x11, 0x9a, 0xa0, // movups %xmm11,0xa0(%rdx)
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x11, 0xa2, 0xb0, // movups %xmm12,0xb0(%rdx)
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x11, 0xaa, 0xc0, // movups %xmm13,0xc0(%rdx)
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x11, 0xb2, 0xd0, // movups %xmm14,0xd0(%rdx)
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x11, 0xba, 0xe0, // movups %xmm15,0xe0(%rdx)
            0x00, 0x00, 0x00,
            0x4c, 0x8b, 0x01, // mov (%rcx),%r8
            0x48, 0x8b, 0x61, 0x08, // mov 0x8(%rcx),%rsp
            0x48, 0x8b, 0x59, 0x10, // mov 0x10(%rcx),%rbx
            0x48, 0x8b, 0x69, 0x18, // mov 0x18(%rcx),%rbp
            0x4c, 0x8b, 0x61, 0x20, // mov 0x20(%rcx),%r12
            0x4c, 0x8b, 0x69, 0x28, // mov 0x28(%rcx),%r13
            0x4c, 0x8b, 0x71, 0x30, // mov 0x30(%rcx),%r14
            0x4c, 0x8b, 0x79, 0x38, // mov 0x38(%rcx),%r15
            0x48, 0x8b, 0x79, 0x40, // mov 0x40(%rcx),%rdi
            0x48, 0x8b, 0x71, 0x48, // mov 0x48(%rcx),%rsi
            0x0f, 0x10, 0x71, 0x50, // movups 0x50(%rcx),%xmm6
            0x0f, 0x10, 0x79, 0x60, // movups 0x60(%rcx),%xmm7
            0x44, 0x0f, 0x10, 0x41, 0x70, // movups 0x70(%rcx),%xmm8
            0x44, 0x0f, 0x10, 0x89, 0x80, // movups 0x80(%rcx),%xmm9
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x10, 0x91, 0x90, // movups 0x90(%rcx),%xmm10
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x10, 0x99, 0xa0, // movups 0xa0(%rcx),%xmm11
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x10, 0xa1, 0xb0, // movups 0xb0(%rcx),%xmm12
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x10, 0xa9, 0xc0, // movups 0xc0(%rcx),%xmm13
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x10, 0xb1, 0xd0, // movups 0xd0(%rcx),%xmm14
            0x00, 0x00, 0x00,
            0x44, 0x0f, 0x10, 0xb9, 0xe0, // movups 0xe0(%rcx),%xmm15
            0x00, 0x00, 0x00,
            0x41, 0x50, // push %r8
            0x31, 0xc0, // xor %eax,%eax
            0xc3 // retq
    };

    using KryneEngine::FiberContext;

#if CONTEXT_SWITCH_ABI_WINDOWS
    void (*_GetContext)(FiberContext*) = (void (*)(FiberContext*))get_context_win_asm;
    void (*_SetContext)(FiberContext*) = (void (*)(FiberContext*))set_context_win_asm;
    void (*_SwapContext)(FiberContext*, FiberContext*) = (void (*)(FiberContext*, FiberContext*))swap_context_win_asm;
#elif CONTEXT_SWITCH_ABI_SYS_V
    void (*_GetContext)(FiberContext*) = (void (*)(FiberContext*))get_context_sysV_asm;
    void (*_SetContext)(FiberContext*) = (void (*)(FiberContext*))set_context_sysV_asm;
    void (*_SwapContext)(FiberContext*, FiberContext*) = (void (*)(FiberContext*, FiberContext*))swap_context_sysV_asm;
#endif
}

namespace KryneEngine
{
    void FiberContext::SwapContext(FiberContext *_new)
    {
#if CONTEXT_SWITCH_WINDOWS_FIBERS
        Assert(m_winFiber == GetCurrentFiber());

        SwitchToFiber((LPVOID*)_new->m_winFiber);
#else
        _SwapContext(_current, _new);
#endif
    }

    [[noreturn]] void FiberContext::RunFiber(void *)
    {
        const auto fibersManager = FibersManager::GetInstance();

        while (true)
        {
            fibersManager->_OnContextSwitched();
            auto* job = fibersManager->GetCurrentJob();

            if (Verify(job->m_status == FiberJob::Status::PendingStart))
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
            for (u16 i = 0; i < kSmallStackCount; i++)
            {
                m_availableSmallContextsIds.enqueue(i);
            }

            for (u16 i = 0; i < kBigStackCount; i++)
            {
                m_availableBigContextsIds.enqueue(kSmallStackCount + i);
            }

#if CONTEXT_SWITCH_WINDOWS_FIBERS

            for (u32 i = 0; i < m_contexts.size(); i++)
            {
                auto& context = m_contexts[i];
                if (i < kSmallStackCount)
                {
                    context.m_winFiber = CreateFiber(kSmallStackSize, FiberContext::RunFiber, nullptr);
                }
                else
                {
                    context.m_winFiber = CreateFiber(kBigStackSize, FiberContext::RunFiber, nullptr);
                }
            }

#elif CONTEXT_SWITCH_ABI_WINDOWS || CONTEXT_SWITCH_ABI_SYS_V

            m_smallStacks = new u8[kSmallStackSize * (u64) kSmallStackCount];
            m_bigStacks = new u8[kBigStackSize * (u64) kBigStackCount];

#endif
        }
    }

    bool FiberContextAllocator::Allocate(bool _bigStack, u16 &id_)
    {
        auto& queue = _bigStack
                ? m_availableBigContextsIds
                : m_availableSmallContextsIds;

        IF_NOT_VERIFY_MSG(queue.try_dequeue(id_), "Out of Fiber stacks!")
        {
            return false;
        }
        return true;
    }

    void FiberContextAllocator::Free(u16 _id)
    {
        VERIFY_OR_RETURN_VOID(_id < m_contexts.size());

        if (_id < kSmallStackCount)
        {
            m_availableSmallContextsIds.enqueue(_id);
        }
        else
        {
            m_availableBigContextsIds.enqueue(_id);
        }
    }

    FiberContext *FiberContextAllocator::GetContext(u16 _id)
    {
        VERIFY_OR_RETURN(_id < m_contexts.size(), nullptr);
        return &m_contexts[_id];
    }
}
