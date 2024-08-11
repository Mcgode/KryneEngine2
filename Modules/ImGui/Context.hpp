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
        explicit Context(GraphicsContext& _graphicsContext, RenderPassHandle _renderPass);
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

        eastl::vector<char> m_vsBytecode {};
        eastl::vector<char> m_fsBytecode {};
        ShaderModuleHandle m_vsModule { GenPool::kInvalidHandle };
        ShaderModuleHandle m_fsModule { GenPool::kInvalidHandle };
        DescriptorSetHandle m_descriptorSet { GenPool::kInvalidHandle };
        eastl::vector<u32> m_setIndices;
        PipelineLayoutHandle m_pipelineLayout { GenPool::kInvalidHandle };
        GraphicsPipelineHandle m_pso { GenPool::kInvalidHandle };

        static constexpr u64 kInitialSize = 1024;
        GraphicsUtils::DynamicBuffer m_dynamicVertexBuffer;
        GraphicsUtils::DynamicBuffer m_dynamicIndexBuffer;

        void _InitPso(GraphicsContext& _graphicsContext, RenderPassHandle _renderPass);
    };
}// namespace KryneEngine