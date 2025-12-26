/**
 * @file
 * @author Max Godefroy
 * @date 08/12/2025.
 */

#pragma once

#include "../../../../../../GraphicsUtils/Include/KryneEngine/Modules/GraphicsUtils/DynamicBuffer.hpp"
#include "KryneEngine/Core/Math/Matrix.hpp"
#include "KryneEngine/Modules/GuiLib/IGuiRenderer.hpp"

namespace KryneEngine::Modules::GuiLib
{

    /**
     * @brief A basic renderer for the GUI that does 1 draw call per UI element
     */
    class BasicGuiRenderer final: public IGuiRenderer
    {
    public:
        BasicGuiRenderer(
            AllocatorInstance _allocator,
            GraphicsContext& _graphicsContext,
            RenderPassHandle _renderPass);

        void BeginLayout(const float4x4& _viewportTransform, const uint2& _viewportSize) override;
        void EndLayoutAndRender(
            GraphicsContext& _graphicsContext,
            CommandListHandle _transferCommandList,
            CommandListHandle _renderCommandList) override;

    private:
        static constexpr u32 kMaxTextureSlots = 32;
        static constexpr u32 kMaxSamplerSlots = 8;

        GraphicsUtils::DynamicBuffer m_instanceDataBuffer;
        GraphicsUtils::DynamicBuffer m_commonConstantBuffer;
        DynamicArray<BufferViewHandle> m_commonConstantBufferViews;

        struct alignas(sizeof(float4)) ViewportConstants
        {
            float4x4 ndcProjectionMatrix;
            float2 viewportSize;
        } m_viewportConstants;

        DescriptorSetHandle m_commonDescriptorSet;
        DescriptorSetHandle m_texturesDescriptorSet;

        PipelineLayoutHandle m_commonPipelineLayout;

        GraphicsPipelineHandle m_rectanglePipeline;
        GraphicsPipelineHandle m_borderPipeline;

        eastl::array<u32, 1> m_commonDescriptorSetIndices {};
        eastl::array<u32, 2> m_texturesDescriptorSetIndices {};
    };

} // namespace KryneEngine
