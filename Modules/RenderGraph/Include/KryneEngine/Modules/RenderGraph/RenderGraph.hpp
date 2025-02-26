/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/unique_ptr.h>

#include <KryneEngine/Core/Common/StringHelpers.hpp>

namespace KryneEngine
{
    class GraphicsContext;
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
        void SubmitFrame(GraphicsContext& _graphicsContext);

    private:
        eastl::unique_ptr<Registry> m_registry;
        eastl::unique_ptr<Builder> m_builder;

        eastl::hash_map<StringHash, u64> m_previousFramePassPerformance;
        eastl::hash_map<StringHash, u64> m_currentFramePassPerformance;
        u64 m_previousFrameTotalDuration = 0;
        u64 m_currentFrameTotalDuration = 0;
    };
} // namespace KryneEngine::Modules::RenderGraph
