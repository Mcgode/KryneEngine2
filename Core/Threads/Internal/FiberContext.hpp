/**
 * @file
 * @author Max Godefroy
 * @date 02/07/2022.
 */

#pragma once

#if defined(_WIN32)
    #define CONTEXT_SWITCH_WINDOWS_FIBERS 1
#elif defined(__APPLE__)
#   define CONTEXT_SWITCH_UCONTEXT 1
#elif __linux__ || defined(__APPLE__) || __unix__
    #define CONTEXT_SWITCH_ABI_SYS_V 1
#else
#error Unsupported ABI
#endif

#if CONTEXT_SWITCH_ABI_WINDOWS
    #include <emmintrin.h>
#endif

#if defined(CONTEXT_SWITCH_UCONTEXT)
#   define _XOPEN_SOURCE
#   include <ucontext.h>
#endif

#include <Common/Types.hpp>
#include <Common/Utils/Alignment.hpp>
#include <EASTL/array.h>
#include <EASTL/priority_queue.h>

namespace KryneEngine
{
    struct FiberContext
    {
        friend struct FiberContextAllocator;

#if CONTEXT_SWITCH_WINDOWS_FIBERS
        void* m_winFiber;
#elif CONTEXT_SWITCH_UCONTEXT
        ucontext_t m_uContext;
#elif CONTEXT_SWITCH_ABI_WINDOWS
        void *rip, *rsp;
        void *rbx, *rbp, *r12, *r13, *r14, *r15, *rdi, *rsi;
        __m128i xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
#elif CONTEXT_SWITCH_ABI_SYS_V
        void *rip, *rsp;
        void *rbx, *rbp, *r12, *r13, *r14, *r15;
#endif

        eastl::string m_name;

        void SwapContext(FiberContext *_new);

    private:
        static void RunFiber();
        static void RunFiber(void*);
    };

    struct FiberContextAllocator
    {
    public:
        FiberContextAllocator();

        ~FiberContextAllocator();

        bool Allocate(bool _bigStack, u16 &id_);

        void Free(u16 _id);

        FiberContext* GetContext(u16 _id);

        static constexpr u16 kSmallStackCount = 128;
        static constexpr u16 kBigStackCount = 32;

    private:
        static constexpr size_t kSmallStackSize = 64 * 1024; // 64 KiB
        static constexpr size_t kBigStackSize = 512 * 1024; // 512 KiB
        static constexpr size_t kStackAlignment = 16;

        static_assert(Alignment::IsAligned(kSmallStackSize, kStackAlignment));
        static_assert(Alignment::IsAligned(kBigStackSize, kStackAlignment));

        struct StackIdQueue
        {
            struct Comparator
            {
                constexpr bool operator()(const u16 _a, const u16 _b) const { return _a > _b; }
            };

            eastl::priority_queue<u16, eastl::vector<u16>, Comparator> m_priorityQueue;
            SpinLock m_spinLock;
        };
        StackIdQueue m_availableSmallContextsIds;
        StackIdQueue m_availableBigContextsIds;

        eastl::array<FiberContext, kSmallStackCount + kBigStackCount> m_contexts {};

#if CONTEXT_SWITCH_UCONTEXT ||  CONTEXT_SWITCH_ABI_WINDOWS || CONTEXT_SWITCH_ABI_SYS_V
        u8* m_smallStacks = nullptr;
        u8* m_bigStacks = nullptr;
#endif
    };
}