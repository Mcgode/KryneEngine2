/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#pragma once

#include <EASTL/unique_ptr.h>

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
    };
} // namespace KryneEngine::Modules::RenderGraph
