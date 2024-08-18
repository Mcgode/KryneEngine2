/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include "Device.hpp"

#include "../Public/RpsRuntime.hpp"

namespace KryneEngine::Modules::RpsRuntime
{
    Device::Device(rps::Device* pDevice, const RuntimeDeviceCreateInfo* pCreateInfo)
        : rps::RuntimeDevice(pDevice, pCreateInfo->m_runtimeCreateInfo)
    {
    }

    RpsResult Device::BuildDefaultRenderGraphPhases(rps::RenderGraph& renderGraph)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Device::InitializeSubresourceInfos(rps::ArrayRef<rps::ResourceInstance> resInstances)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Device::InitializeResourceAllocInfos(rps::ArrayRef<rps::ResourceInstance> resInstances)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Device::GetSubresourceRangeFromImageView(
        rps::SubresourceRangePacked& outRange,
        const rps::ResourceInstance& resourceInfo,
        const RpsAccessAttr& accessAttr,
        const RpsImageView& imageView)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }
} // namespace KryneEngine::Modules::RpsRuntime