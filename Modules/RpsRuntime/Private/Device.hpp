/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#pragma once

#include <runtime/common/rps_runtime_device.hpp>

namespace KryneEngine::Modules::RpsRuntime
{
    struct RuntimeDeviceCreateInfo;

    class Device : public rps::RuntimeDevice
    {
    public:
        Device(rps::Device* pDevice, const RuntimeDeviceCreateInfo* pCreateInfo);

        RpsResult BuildDefaultRenderGraphPhases(rps::RenderGraph& renderGraph) final;

        RpsResult InitializeSubresourceInfos(rps::ArrayRef<rps::ResourceInstance> resInstances) final;

        RpsResult InitializeResourceAllocInfos(rps::ArrayRef<rps::ResourceInstance> resInstances) final;

        RpsResult GetSubresourceRangeFromImageView(
            rps::SubresourceRangePacked& outRange,
            const rps::ResourceInstance& resourceInfo,
            const RpsAccessAttr& accessAttr,
            const RpsImageView& imageView) final;
    };
} // namespace KryneEngine::Modules::RpsRuntime