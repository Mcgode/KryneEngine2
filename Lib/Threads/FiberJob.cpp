/**
 * @file
 * @author Max Godefroy
 * @date 02/07/2022.
 */

#include "FiberJob.hpp"
#include <Threads/FibersManager.hpp>

namespace KryneEngine
{
    FiberJob::FiberJob()
    {
        m_context.rsp = nullptr;
        m_context.rip = (void*)_KickJob;
    }

    void FiberJob::_SetStackPointer(u16 _stackId, u8 *_stackPtr, u64 _stackSize)
    {
        m_stackId = _stackId;

        // Stacks grow downwards, so we need a pointer to the end of the stack.
        u8* stackPtr = _stackPtr + _stackSize;

        // Align stack pointer on 16-byte boundary.
        stackPtr = (u8*)(((u64)m_context.rsp & -16L));

#if CONTEXT_SWITCH_ABI_WINDOWS
        // Windows has no concept of scratch/red zone:
        // https://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64
#elif CONTEXT_SWITCH_ABI_SYS_V
        // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
        // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
        // 16-byte aligned.
        stackPtr -= 128;
#endif

        m_context.rsp = stackPtr;
    }

    void FiberJob::_ResetStackPointer()
    {
        m_stackId = kInvalidStackId;
        m_context.rsp = nullptr;
    }

    void FiberJob::_KickJob()
    {
        const auto fibersManager = FibersManager::GetInstance();

        fibersManager->_OnContextSwitched();
        auto* job = fibersManager->GetCurrentJob();

        if (!Verify(job->m_status == Status::PendingStart))
        {
            return;
        }

        job->m_status = Status::Running;
        job->m_functionPtr(job->m_userData);
        job->m_status = Status::Finished;

        fibersManager->YieldJob();
    }
} // KryneEngine