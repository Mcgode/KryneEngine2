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
}
