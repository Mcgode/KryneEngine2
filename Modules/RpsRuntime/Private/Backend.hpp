/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#pragma once

#include <runtime/common/rps_render_graph.hpp>

namespace KryneEngine::Modules::RpsRuntime
{
    class Backend: public rps::RuntimeBackend
    {
    public:
        explicit Backend(rps::RenderGraph& _renderGraph);

        RpsResult CreateHeaps(
            const rps::RenderGraphUpdateContext& _context,
            rps::ArrayRef<rps::HeapInfo> _heaps) final;

        [[nodiscard]] RpsResult RecordCommands(
            const rps::RenderGraph& _renderGraph,
            const RpsRenderGraphRecordCommandInfo& _recordInfo) const final;

        void DestroyRuntimeResourceDeferred(rps::ResourceInstance& resource) final;
    };
} // namespace KryneEngine::Modules::RpsRuntime