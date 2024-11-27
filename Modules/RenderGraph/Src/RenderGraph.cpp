/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#include "KryneEngine/Modules/RenderGraph/RenderGraph.hpp"

#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>

#include "KryneEngine/Modules/RenderGraph/Builder.hpp"
#include "KryneEngine/Modules/RenderGraph/Registry.hpp"

namespace KryneEngine::Modules::RenderGraph
{
    struct PassExecutionData
    {
        CommandListHandle m_commandList;
    };

    RenderGraph::RenderGraph()
    {
        m_registry = eastl::make_unique<Registry>();
    }

    RenderGraph::~RenderGraph() = default;

    Builder& RenderGraph::BeginFrame(GraphicsContext& _graphicsContext)
    {
        m_builder = eastl::make_unique<Builder>(GetRegistry());
        return *m_builder;
    }

    void RenderGraph::SubmitFrame(GraphicsContext& _graphicsContext)
    {
        m_builder->PrintBuildResult();

        PassExecutionData passExecutionData = {
            .m_commandList = {},
        };

        for (size_t i = 0; i < m_builder->m_declaredPasses.size(); i++)
        {
            if (!m_builder->m_passAlive[i])
            {
                continue;
            }

            KE_ASSERT(m_builder->m_declaredPasses[i].m_executeFunction != nullptr);
            m_builder->m_declaredPasses[i].m_executeFunction(*this, passExecutionData);
        }

        m_builder.reset();
    }
} // namespace KryneEngine::Modules::RenderGraph