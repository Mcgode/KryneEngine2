/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include <thread>
#include <Common/KETypes.hpp>

namespace KryneEngine::Threads
{
    bool SetThreadHardwareAffinity(std::thread& _thread, u32 _coreIndex);

    bool DisableThreadSignals();

    inline void CpuYield()
    {
#if defined(__x86_64__) || defined(__i386__)
        _mm_pause();
#elif defined(__arm__)
        __yield()
#else
#warning No CPU pause/yield instruction
#endif
    }
}
