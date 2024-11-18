/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#include "RenderGraph.hpp"

#include <RenderGraph/Builder.hpp>
#include <RenderGraph/Registry.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    RenderGraph::RenderGraph()
    {
        m_registry = eastl::make_unique<Registry>();
    }

    RenderGraph::~RenderGraph() = default;

    Builder& RenderGraph::BeginFrame()
    {
        m_builder = eastl::make_unique<Builder>(GetRegistry());
        return *m_builder;
    }

    void RenderGraph::SubmitFrame()
    {
        m_builder->PrintBuildResult();
        m_builder.reset();
    }
} // namespace KryneEngine::Modules::RenderGraph