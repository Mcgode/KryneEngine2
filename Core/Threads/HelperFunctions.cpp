/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#include "HelperFunctions.hpp"

#if defined(_WIN32) || defined(WIN32)
#   define WINDOWS_THREADS
#   include <Platform/Windows.h>
#elif defined(__unix__)
    #define PTHREADS
    #include <pthread.h>
#elif defined(__APPLE__)
#   define MACOS_THREADS
#   include <mach/mach.h>
#   include <mach/thread_policy.h>
#   include <pthread.h>
#   include <sched.h>
#endif

#include <Common/Assert.hpp>

namespace KryneEngine::Threads
{
    bool SetThreadHardwareAffinity(std::thread &_thread, u32 _coreIndex)
    {
#if defined(WINDOWS_THREADS)
        DWORD_PTR dw = SetThreadAffinityMask(_thread.native_handle(), DWORD_PTR(1) << _coreIndex);
        if (dw == 0)
        {
            eastl::string msg;
            msg.sprintf("Unable to set thread affinity mask: %s", GetLastError());
            KE_ERROR(msg.c_str());
        }
        return dw != 0;
#elif defined(PTHREADS)
        cpu_set_t coreSet {};
        CPU_ZERO(&coreSet);
        CPU_SET(_coreIndex, &coreSet);
        s32 result = pthread_setaffinity_np(_thread.native_handle(), 1, &coreSet);
        return result == 0;
#elif defined(MACOS_THREADS)
        const pthread_t thread = _thread.native_handle();
        thread_t threadPort = pthread_mach_thread_np(thread);
        thread_affinity_policy_data_t policy;
        policy.affinity_tag = static_cast<s32>(_coreIndex);
        const s32 result = thread_policy_set(
            threadPort,
            THREAD_AFFINITY_POLICY,
            reinterpret_cast<thread_policy_t>(&policy),
            1);
        return result == 0;
#else
        #error No supported thread API
        return false;
#endif
    }

    bool DisableThreadSignals()
    {
#if defined(WINDOWS_THREADS)
        return true;
#elif defined(PTHREADS) || defined(MACOS_THREADS)
        sigset_t mask;
        sigfillset(&mask);
        return pthread_sigmask(SIG_BLOCK, &mask, nullptr) == 0;
#else
#error No supported thread API
        return false;
#endif
    }
}