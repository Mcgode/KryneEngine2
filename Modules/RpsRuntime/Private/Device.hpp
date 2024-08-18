/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#pragma once

#include <runtime/common/rps_runtime_device.hpp>

namespace KryneEngine
{
    class GraphicsContext;
}

namespace KryneEngine::Modules::RpsRuntime
{
    struct RuntimeDeviceCreateInfo;

    class Device : public rps::RuntimeDevice
    {
    public:
        Device(rps::Device* _pDevice, const RuntimeDeviceCreateInfo* _createInfo);

        RpsResult BuildDefaultRenderGraphPhases(rps::RenderGraph& _renderGraph) final;

        RpsResult InitializeSubresourceInfos(rps::ArrayRef<rps::ResourceInstance> _resInstances) final;

        RpsResult InitializeResourceAllocInfos(rps::ArrayRef<rps::ResourceInstance> _resInstances) final;

        RpsResult GetSubresourceRangeFromImageView(
            rps::SubresourceRangePacked& _outRange,
            const rps::ResourceInstance& _resourceInfo,
            const RpsAccessAttr& _accessAttr,
            const RpsImageView& _imageView) final;

        [[nodiscard]] GraphicsContext* GetGraphicsContext() const { return m_graphicsContext; }

    private:
        GraphicsContext* m_graphicsContext;
    };
} // namespace KryneEngine::Modules::RpsRuntime