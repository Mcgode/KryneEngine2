/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#pragma once

#include <imgui.h>
#include <Graphics/Common/GraphicsContext.hpp>
#include <GraphicsUtils/DynamicBuffer.hpp>

namespace KryneEngine
{
    class Window;
}

namespace KryneEngine::Modules::ImGui
{
    /**
     * @class Context
     *
     * This class represents the rendering and input handling context for ImGui (Immediate Mode GUI).
     */
    class Context
    {
    public:
        /**
         * @brief Constructs a Context object.
         *
         * This constructor initializes the ImGui context by calling ImGui::CreateContext().
         * It also sets up the ImGuiIO data structure and initializes the vertex and index dynamic buffers.
         *
         * @param _window The Window object associated with the Context.
         * @param _renderPass The RenderPassHandle object used for building the ImGui PSO.
         */
        explicit Context(Window* _window, RenderPassHandle _renderPass);

        ~Context();

        /**
         * @brief Shut down the context.
         *
         * @param _graphicsContext The graphics context that owns the graphic resources.
         */
        void Shutdown(GraphicsContext* _graphicsContext);

        /**
         * Sets up the ImGui context for a new frame.
         * Updates input and window data.
         *
         * @param _window The Window object.
         * @param _commandList The command list.
         */
        void NewFrame(Window* _window, CommandList _commandList);

        /**
         * @brief Prepares the rendering context for a new frame by updating the vertex and index buffers.
         *
         * @param _graphicsContext The graphics context used for rendering.
         * @param _commandList The command list used for uploading the buffers.
         */
        void PrepareToRenderFrame(GraphicsContext* _graphicsContext, CommandList _commandList);

        /**
         * @brief Renders a frame using the provided graphics context and command list.
         *
         * This function is responsible for rendering the ImGui UI for a single frame.
         *
         * @param _graphicsContext The graphics context used for rendering.
         * @param _commandList The command list used for rendering.
         */
        void RenderFrame(GraphicsContext* _graphicsContext, CommandList _commandList);

    private:
        ImGuiContext* m_context;
        u64 m_stagingFrame = 0;
        BufferHandle m_fontsStagingHandle { GenPool::kInvalidHandle };
        TextureHandle m_fontsTextureHandle { GenPool::kInvalidHandle };
        TextureSrvHandle m_fontTextureSrvHandle{ GenPool::kInvalidHandle };
        SamplerHandle m_fontSamplerHandle { GenPool::kInvalidHandle };

        DescriptorSetLayoutHandle m_fontDescriptorSetLayout { GenPool::kInvalidHandle };
        DescriptorSetHandle m_fontDescriptorSet { GenPool::kInvalidHandle };

        eastl::vector<char> m_vsBytecode {};
        eastl::vector<char> m_fsBytecode {};
        ShaderModuleHandle m_vsModule { GenPool::kInvalidHandle };
        ShaderModuleHandle m_fsModule { GenPool::kInvalidHandle };
        eastl::vector<u32> m_setIndices;
        PipelineLayoutHandle m_pipelineLayout { GenPool::kInvalidHandle };
        GraphicsPipelineHandle m_pso { GenPool::kInvalidHandle };

        static constexpr u64 kInitialSize = 1024;
        GraphicsUtils::DynamicBuffer m_dynamicVertexBuffer;
        GraphicsUtils::DynamicBuffer m_dynamicIndexBuffer;

        void _InitPso(GraphicsContext* _graphicsContext, RenderPassHandle _renderPass);
    };
}// namespace KryneEngine