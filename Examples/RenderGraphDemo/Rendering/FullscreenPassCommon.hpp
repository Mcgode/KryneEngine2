/**
 * @file
 * @author Max Godefroy
 * @date 10/04/2025.
 */

#pragma once

#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>

namespace KryneEngine::Samples::RenderGraphDemo::FullscreenPassCommon
{
    [[nodiscard]] extern GraphicsPipelineHandle CreatePso(
        GraphicsContext* _graphicsContext,
        AllocatorInstance _allocator,
        RenderPassHandle _renderPass,
        PipelineLayoutHandle _pipelineLayout,
        const char* _psShader);

    void Render(
        GraphicsContext* _graphicsContext,
        CommandListHandle _commandList,
        u32 _width,
        u32 _height,
        float _fullscreenDepth,
        GraphicsPipelineHandle _pso,
        PipelineLayoutHandle _pipelineLayout,
        eastl::span<DescriptorSetHandle> _descriptorSets);
}
