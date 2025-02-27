/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/unique_ptr.h>

#include "KryneEngine/Modules/RenderGraph/Declarations/PassDeclaration.hpp"

namespace KryneEngine
{
    class GraphicsContext;
    class FibersManager;
}

namespace KryneEngine::Modules::RenderGraph
{
    class Builder;
    class Registry;

    class RenderGraph
    {
    public:
        RenderGraph();
        ~RenderGraph();

        [[nodiscard]] Registry& GetRegistry() const { return *m_registry; }
        [[nodiscard]] Builder& GetBuilder() const { return *m_builder; }

        [[nodiscard]] Builder& BeginFrame(GraphicsContext& _graphicsContext);
        void SubmitFrame(GraphicsContext& _graphicsContext, FibersManager& _fibersManager);

    private:
        eastl::unique_ptr<Registry> m_registry;
        eastl::unique_ptr<Builder> m_builder;

        struct JobData
        {
            RenderGraph* m_renderGraph;
            GraphicsContext* m_graphicsContext;
            PassExecutionData m_passExecutionData;
            u32 m_passRangeStart;
            u32 m_passRangeCount;
        };
        eastl::vector<JobData> m_jobs;

        eastl::hash_map<StringHash, u64> m_previousFramePassPerformance;
        eastl::hash_map<StringHash, u64> m_currentFramePassPerformance;
        u64 m_previousFrameTotalDuration = 0;
        std::atomic<u64> m_currentFrameTotalDuration = 0;

        static void ExecuteJob(void* _userData);
    };
} // namespace KryneEngine::Modules::RenderGraph
