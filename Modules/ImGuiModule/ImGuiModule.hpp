/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#pragma once

#include <imgui.h>
#include <Graphics/Common/GraphicsContext.hpp>

namespace KryneEngine
{
    class ImGuiModule
    {
    public:
        explicit ImGuiModule(GraphicsContext &_graphicsContext);
        ~ImGuiModule();

        void Shutdown(GraphicsContext& _graphicsContext);

        void NewFrame(GraphicsContext& _graphicsContext, CommandList _commandList);

        void PrepareToRenderFrame(GraphicsContext& _graphicsContext, CommandList _commandList);
        void RenderFrame(GraphicsContext& _graphicsContext, CommandList _commandList);

    private:
        ImGuiContext* m_context;
        u64 m_stagingFrame = 0;
        BufferHandle m_fontsStagingHandle { GenPool::kInvalidHandle };
        TextureHandle m_fontsTextureHandle { GenPool::kInvalidHandle };
        TextureSrvHandle m_fontsTextureSrvHandle { GenPool::kInvalidHandle };
    };
}// namespace KryneEngine