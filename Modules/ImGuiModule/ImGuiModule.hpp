/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#pragma once

struct ImGuiContext;

namespace KryneEngine
{
    class GraphicsContext;

    class ImGuiModule
    {
    public:
        explicit ImGuiModule(GraphicsContext &_graphicsContext);
        ~ImGuiModule();

        void Shutdown(GraphicsContext& _graphicsContext);

        void NewFrame(GraphicsContext& _graphicsContext);

        void RenderFrame(GraphicsContext& _graphicsContext);

    private:
        ImGuiContext* m_context;
    };
}// namespace KryneEngine