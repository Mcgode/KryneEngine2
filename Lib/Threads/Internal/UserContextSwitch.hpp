/**
 * @file
 * @author Max Godefroy
 * @date 02/07/2022.
 */

#pragma once

#if defined(_WIN32)
    #define CONTEXT_SWITCH_ABI_WINDOWS 1
#elif __linux__ || defined(__APPLE__) || __unix__
    #define CONTEXT_SWITCH_ABI_SYS_V 1
#else
#error Unsupported ABI
#endif

#if CONTEXT_SWITCH_ABI_WINDOWS
    #include <emmintrin.h>
#endif

namespace KryneEngine
{
    struct FiberContext
    {
#if CONTEXT_SWITCH_ABI_WINDOWS
        void *rip, *rsp;
        void *rbx, *rbp, *r12, *r13, *r14, *r15, *rdi, *rsi;
        __m128i xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
#elif CONTEXT_SWITCH_ABI_SYS_V
        void *rip, *rsp;
        void *rbx, *rbp, *r12, *r13, *r14, *r15;
#endif
    };

    void GetContext(FiberContext* _current);
    void SetContext(FiberContext* _new);
    void SwapContext(FiberContext* _current, FiberContext* _new);
}