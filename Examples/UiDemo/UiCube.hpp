/**
 * @file
 * @author Max Godefroy
 * @date 14/01/2026.
 */

#pragma once

#include <KryneEngine/Core/Graphics/GraphicsContext.hpp>
#include <KryneEngine/Core/Graphics/Handles.hpp>
#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>
#include <KryneEngine/Modules/GraphicsUtils/DynamicBuffer.hpp>
#include <KryneEngine/Modules/GuiLib/Context.hpp>
#include <KryneEngine/Modules/GuiLib/GuiRenderers/BasicGuiRenderer.hpp>
#include <KryneEngine/Modules/TextRendering/Font.hpp>

using namespace KryneEngine;

class UiCube
{
public:
    UiCube(
        AllocatorInstance _allocator,
        GraphicsContext& _graphicsContext,
        Modules::TextRendering::FontManager* _fontManager,
        RenderPassHandle _renderPass);

    void Render(GraphicsContext& _graphicsContext, CommandListHandle _transferCommandList, CommandListHandle _renderCommandList);

private:
    AllocatorInstance m_allocator;

    Modules::GuiLib::Context m_guiContext;
    Modules::GuiLib::BasicGuiRenderer m_guiRenderer;

    BufferHandle m_vertexBuffer;
    BufferHandle m_indexBuffer;
    BufferHandle m_transferBuffer;
    u32 m_transferFrameId = ~0u;

    struct UiCubeData
    {
        float4x4 m_mvpMatrix;
    };

    Modules::GraphicsUtils::DynamicBuffer m_constantBuffer;
    DynamicArray<BufferViewHandle> m_constantBufferViews;

    DescriptorSetHandle m_descriptorSet;
    u32 m_descriptorSetIndex;
    PipelineLayoutHandle m_pipelineLayout;
    GraphicsPipelineHandle m_pso;
};
