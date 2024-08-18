/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include "Backend.hpp"

#include "Device.hpp"

namespace KryneEngine::Modules::RpsRuntime
{
    Backend::Backend(Device& _device, rps::RenderGraph& _renderGraph)
        : rps::RuntimeBackend(_renderGraph)
        , m_device(_device)
    {}

    RpsResult Backend::RecordCommands(
        const rps::RenderGraph& renderGraph, const RpsRenderGraphRecordCommandInfo& recordInfo) const
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Backend::RecordCmdRenderPassBegin(const rps::RuntimeCmdCallbackContext& context) const
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Backend::RecordCmdRenderPassEnd(const rps::RuntimeCmdCallbackContext& context) const
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Backend::RecordCmdFixedFunctionBindingsAndDynamicStates(const rps::RuntimeCmdCallbackContext& context) const
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    void Backend::DestroyRuntimeResourceDeferred(rps::ResourceInstance& resource)
    {
        KE_ERROR("Not implemented");
    }

    RpsResult Backend::UpdateFrame(const rps::RenderGraphUpdateContext& context)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Backend::CreateHeaps(const rps::RenderGraphUpdateContext& context, rps::ArrayRef<rps::HeapInfo> heaps)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    void Backend::DestroyHeaps(rps::ArrayRef<rps::HeapInfo> heaps)
    {
        KE_ERROR("Not implemented");
        RuntimeBackend::DestroyHeaps(heaps);
    }

    RpsResult Backend::CreateResources(const rps::RenderGraphUpdateContext& context, rps::ArrayRef<rps::ResourceInstance> resources)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    void Backend::DestroyResources(rps::ArrayRef<rps::ResourceInstance> resources)
    {
        KE_ERROR("Not implemented");
        RuntimeBackend::DestroyResources(resources);
    }

    RpsResult Backend::CreateCommandResources(const rps::RenderGraphUpdateContext& context)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    void Backend::DestroyCommandResources()
    {
        KE_ERROR("Not implemented");
        RuntimeBackend::DestroyCommandResources();
    }

    void Backend::RecordDebugMarker(
        const rps::RuntimeCmdCallbackContext& context,
        RpsRuntimeDebugMarkerMode mode, rps::StrRef name) const
    {
        KE_ERROR("Not implemented");
        RuntimeBackend::RecordDebugMarker(context, mode, name);
    }

    bool Backend::ShouldResetAliasedResourcesPrevFinalAccess() const
    {
        KE_ERROR("Not implemented");
        return RuntimeBackend::ShouldResetAliasedResourcesPrevFinalAccess();
    }
} // namespace KryneEngine::Modules::RpsRuntime