/**
 * @file
 * @author Max Godefroy
 * @date 30/04/2025.
 */

#include "ColorMappingPass.hpp"

#include "FullscreenPassCommon.hpp"

#include <KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp>

namespace KryneEngine::Samples::RenderGraphDemo
{
    ColorMappingPass::ColorMappingPass(AllocatorInstance _allocator)
        : m_allocator(_allocator)
    {
        static_assert(
            offsetof(ColorMappingPass, m_sceneConstantsDescriptorSet) + sizeof(DescriptorSetHandle)
                == offsetof(ColorMappingPass, m_inputColorDescriptorSet),
            "The two descriptor sets must be contiguous in memory, to load them in the same 2-wide span");
    }

    ColorMappingPass::~ColorMappingPass() = default;

    void ColorMappingPass::Initialize(
        GraphicsContext* _graphicsContext,
        DescriptorSetLayoutHandle _sceneConstantsDescriptorSetLayout,
        TextureSrvHandle _hdrSrv)
    {
        u32 indices[1];

        // Create input color descriptor set layout
        {
            DescriptorSetDesc desc {
                .m_bindings = {
                    DescriptorBindingDesc {
                        .m_type = DescriptorBindingDesc::Type::SampledTexture,
                        .m_visibility = ShaderVisibility::Fragment
                    }, // Input color
                }
            };

            m_inputColorDescriptorSetLayout = _graphicsContext->CreateDescriptorSetLayout(desc, indices);
        }

        // Create and fill input color descriptor set
        {
            m_inputColorDescriptorSet = _graphicsContext->CreateDescriptorSet(m_inputColorDescriptorSetLayout);

            const DescriptorSetWriteInfo writeInfo[] = {
                {
                    .m_index = indices[0],
                    .m_descriptorData = { DescriptorSetWriteInfo::DescriptorData{
                        .m_textureLayout = TextureLayout::ShaderResource,
                        .m_handle = _hdrSrv.m_handle,
                    } }
                },
            };

            _graphicsContext->UpdateDescriptorSet(m_inputColorDescriptorSet, writeInfo);
        }

        // Create PSO layout
        {
            PipelineLayoutDesc layoutDesc = {
                .m_descriptorSets = {
                    _sceneConstantsDescriptorSetLayout,
                    m_inputColorDescriptorSetLayout
                },
                .m_pushConstants = {
                    PushConstantDesc {
                        .m_sizeInBytes = sizeof(u32),
                        .m_visibility = ShaderVisibility::Vertex,
                    }
                }
            };

            m_pipelineLayout = _graphicsContext->CreatePipelineLayout(layoutDesc);
        }
    }

    void ColorMappingPass::Render(
        const Modules::RenderGraph::RenderGraph&,
        const Modules::RenderGraph::PassExecutionData& _passExecutionData)
    {
        if (m_pso == GenPool::kInvalidHandle)
            return;

        FullscreenPassCommon::Render(
            _passExecutionData.m_graphicsContext,
            _passExecutionData.m_commandList,
            _passExecutionData.m_graphicsContext->GetApplicationInfo().m_displayOptions.m_width,
            _passExecutionData.m_graphicsContext->GetApplicationInfo().m_displayOptions.m_height,
            1.f,
            m_pso,
            m_pipelineLayout,
            { &m_sceneConstantsDescriptorSet, 2 }); // Both handles are contiguous in memory
    }

    void ColorMappingPass::CreatePso(GraphicsContext* _graphicsContext, RenderPassHandle _renderPass)
    {
        if (m_pso != GenPool::kInvalidHandle)
            return;

        m_pso = FullscreenPassCommon::CreatePso(
            _graphicsContext,
            m_allocator,
            _renderPass,
            m_pipelineLayout,
            "Shaders/ColorMapping_ColorMappingMain",
            "ColorMappingMain",
            false);
    }
}