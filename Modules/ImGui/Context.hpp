/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#pragma once

#include <imgui.h>
#include <Graphics/Common/GraphicsContext.hpp>
#include <GraphicsUtils/DynamicBuffer.hpp>

namespace KryneEngine::Modules::ImGui
{
    class Context
    {
    public:
        explicit Context(GraphicsContext &_graphicsContext);
        ~Context();

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

        static constexpr u64 kInitialSize = 1024;
        GraphicsUtils::DynamicBuffer m_dynamicVertexBuffer;
        GraphicsUtils::DynamicBuffer m_dynamicIndexBuffer;
    };
}// namespace KryneEngine