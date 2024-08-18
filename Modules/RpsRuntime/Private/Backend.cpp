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

    RpsResult Backend::CreateHeaps(const rps::RenderGraphUpdateContext& _context, rps::ArrayRef<rps::HeapInfo> _heaps)
    {
        return RuntimeBackend::CreateHeaps(_context, _heaps);
    }

    RpsResult Backend::RecordCommands(
        const rps::RenderGraph& _renderGraph, const RpsRenderGraphRecordCommandInfo& _recordInfo) const
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    void Backend::DestroyRuntimeResourceDeferred(rps::ResourceInstance& resource)
    {}
} // namespace KryneEngine::Modules::RpsRuntime