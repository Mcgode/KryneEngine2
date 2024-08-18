/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#pragma once

#include <runtime/common/rps_render_graph.hpp>

namespace KryneEngine::Modules::RpsRuntime
{
    class Device;

    class Backend: public rps::RuntimeBackend
    {
    public:
        explicit Backend(Device& _device, rps::RenderGraph& _renderGraph);

        [[nodiscard]] RpsResult RecordCommands(
            const rps::RenderGraph& renderGraph, const RpsRenderGraphRecordCommandInfo& recordInfo) const override;

        [[nodiscard]] RpsResult RecordCmdRenderPassBegin(const rps::RuntimeCmdCallbackContext& context) const override;
        [[nodiscard]] RpsResult RecordCmdRenderPassEnd(const rps::RuntimeCmdCallbackContext& context) const override;

        [[nodiscard]] RpsResult RecordCmdFixedFunctionBindingsAndDynamicStates(
            const rps::RuntimeCmdCallbackContext& context) const override;

        void DestroyRuntimeResourceDeferred(rps::ResourceInstance& resource) override;

    protected:
        RpsResult UpdateFrame(const rps::RenderGraphUpdateContext& context) override;

        RpsResult CreateHeaps(
            const rps::RenderGraphUpdateContext& context,
            rps::ArrayRef<rps::HeapInfo> heaps) override;
        void DestroyHeaps(rps::ArrayRef<rps::HeapInfo> heaps) override;

        RpsResult CreateResources(
            const rps::RenderGraphUpdateContext& context,
            rps::ArrayRef<rps::ResourceInstance> resources) override;
        void DestroyResources(rps::ArrayRef<rps::ResourceInstance> resources) override;

        RpsResult CreateCommandResources(const rps::RenderGraphUpdateContext& context) override;
        void DestroyCommandResources() override;

        void RecordDebugMarker(
            const rps::RuntimeCmdCallbackContext& context,
            RpsRuntimeDebugMarkerMode mode,
            rps::StrRef name) const override;

        bool ShouldResetAliasedResourcesPrevFinalAccess() const override;

    private:
        Device& m_device;
    };
} // namespace KryneEngine::Modules::RpsRuntime