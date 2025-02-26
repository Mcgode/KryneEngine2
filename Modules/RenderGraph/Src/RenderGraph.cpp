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
            .m_commandList = _graphicsContext.BeginGraphicsCommandList(),
        };

        for (size_t i = 0; i < m_builder->m_declaredPasses.size(); i++)
        {
            if (!m_builder->m_passAlive[i])
            {
                continue;
            }

            const PassDeclaration& pass = m_builder->m_declaredPasses[i];

            std::chrono::time_point start = std::chrono::steady_clock::now();
            KE_ASSERT(pass.m_executeFunction != nullptr);
            pass.m_executeFunction(*this, passExecutionData);
            std::chrono::time_point end = std::chrono::steady_clock::now();

            const u64 duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            m_currentFramePassPerformance.emplace(pass.m_name, duration);
            m_currentFrameTotalDuration += duration;
        }

        _graphicsContext.EndGraphicsCommandList(passExecutionData.m_commandList);

        eastl::swap(m_previousFramePassPerformance, m_currentFramePassPerformance);
        m_currentFramePassPerformance.clear();

        m_previousFrameTotalDuration = m_currentFrameTotalDuration;
        m_currentFrameTotalDuration = 0;

        m_builder.reset();
    }
} // namespace KryneEngine::Modules::RenderGraph