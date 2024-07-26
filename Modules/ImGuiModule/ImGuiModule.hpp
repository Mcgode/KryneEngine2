/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#pragma once

#include <imgui.h>

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
        u64 m_stagingFrame = 0;
        GenPool::Handle m_fontsStagingHandle = GenPool::kInvalidHandle;
        GenPool::Handle m_fontsTextureHandle = GenPool::kInvalidHandle;
    };
}// namespace KryneEngine