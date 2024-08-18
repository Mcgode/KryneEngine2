/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include "../Public/RpsRuntime.hpp"

#include "Device.hpp"

#include <Common/Assert.hpp>
#include <runtime/common/rps_runtime_device.hpp>

namespace KryneEngine::Modules::RpsRuntime
{
    RpsResult rpsRuntimeDeviceCreate(const RuntimeDeviceCreateInfo* _createInfo, RpsDevice* _pDevice)
    {
        VERIFY_OR_RETURN(_createInfo != nullptr && _createInfo->m_graphicsContext != nullptr, RPS_ERROR_INVALID_DATA);

        return rps::RuntimeDevice::Create<Device>(
            _pDevice,
            _createInfo->m_deviceCreateInfo, _createInfo);
    }
}