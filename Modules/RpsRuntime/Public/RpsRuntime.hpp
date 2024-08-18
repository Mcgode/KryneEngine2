/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#pragma once

#include <rps/core/rps_api.h>
#include <rps/runtime/common/rps_runtime.h>

namespace KryneEngine
{
    class GraphicsContext;
}

namespace KryneEngine::Modules::RpsRuntime
{
    struct RuntimeDeviceCreateInfo
    {
        /// Pointer to general RPS device creation parameters. Passing NULL uses default parameters instead.
        const RpsDeviceCreateInfo* m_deviceCreateInfo;

        /// Pointer to general RPS runtime creation info. Passing NULL uses default parameters instead.
        const RpsRuntimeDeviceCreateInfo* m_runtimeCreateInfo;

        /// Pointer to the D3D12 device to use for the runtime. Must not be NULL.
        GraphicsContext* m_graphicsContext;
    };

    RpsResult rpsRuntimeDeviceCreate(const RuntimeDeviceCreateInfo* _createInfo, RpsDevice* _pDevice);
}