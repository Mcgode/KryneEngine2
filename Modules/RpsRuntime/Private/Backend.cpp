/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include "Backend.hpp"

namespace KryneEngine::Modules::RpsRuntime
{
    Backend::Backend(rps::RenderGraph& _renderGraph)
        : RuntimeBackend(_renderGraph)
    {}

    RpsResult Backend::CreateHeaps(const rps::RenderGraphUpdateContext& _context, rps::ArrayRef<rps::HeapInfo> _heaps)
    {
        return RuntimeBackend::CreateHeaps(_context, _heaps);
    }

    RpsResult Backend::RecordCommands(
        const rps::RenderGraph& _renderGraph, const RpsRenderGraphRecordCommandInfo& _recordInfo) const
    {
        return RPS_ERROR_UNKNOWN_NODE;
    }

    void Backend::DestroyRuntimeResourceDeferred(rps::ResourceInstance& resource)
    {}
} // namespace KryneEngine::Modules::RpsRuntime