/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2025.
 */

#include "DeferredShadingPass.hpp"
#include "FullscreenPassCommon.hpp"

#include "KryneEngine/Core/Graphics/Drawing.hpp"
#include "KryneEngine/Core/Graphics/ShaderPipeline.hpp"
#include <fstream>

namespace KryneEngine::Samples::RenderGraphDemo
{

    DeferredShadingPass::DeferredShadingPass(AllocatorInstance _allocator)
        : m_allocator(_allocator)
    {
        static_assert(
            offsetof(DeferredShadingPass, m_sceneConstantsDescriptorSet) + sizeof(DescriptorSetHandle)
            == offsetof(DeferredShadingPass, m_textureDescriptors),
            "The two descriptor sets must be contiguous in memory, to load them in the same 2-wide span");
    }

    void DeferredShadingPass::Initialize(
        GraphicsContext* _graphicsContext,
        DescriptorSetLayoutHandle _sceneConstantsDescriptorSetLayout,
        TextureSrvHandle _gBufferAlbedo,
        TextureSrvHandle _gBufferNormal,
        TextureSrvHandle _gBufferDepth,
        TextureSrvHandle _gBufferAmbient,
        TextureSrvHandle _deferredShadows)
    {
        u32 indices[5];

        // Create texture descriptor set layout
        {
            DescriptorSetDesc desc {
                .m_bindings = {
                    DescriptorBindingDesc {
                        .m_type = DescriptorBindingDesc::Type::SampledTexture,
                        .m_visibility = ShaderVisibility::Fragment
                    }, // Albedo
                    DescriptorBindingDesc {
                        .m_type = DescriptorBindingDesc::Type::SampledTexture,
                        .m_visibility = ShaderVisibility::Fragment
                    }, // Normal
                    DescriptorBindingDesc {
                        .m_type = DescriptorBindingDesc::Type::SampledTexture,
                        .m_visibility = ShaderVisibility::Fragment
                    }, // Depth
                    DescriptorBindingDesc {
                        .m_type = DescriptorBindingDesc::Type::SampledTexture,
                        .m_visibility = ShaderVisibility::Fragment
                    }, // Ambient
                    DescriptorBindingDesc {
                        .m_type = DescriptorBindingDesc::Type::SampledTexture,
                        .m_visibility = ShaderVisibility::Fragment
                    }, // Deferred shadows
                }
            };

            m_texturesDescriptorSetLayout = _graphicsContext->CreateDescriptorSetLayout(desc, indices);
        }

        // Create and fill textures descriptor set
        {
            m_textureDescriptors = _graphicsContext->CreateDescriptorSet(m_texturesDescriptorSetLayout);

            const DescriptorSetWriteInfo writeInfo[] = {
                {
                    .m_index = indices[0],
                    .m_descriptorData = { DescriptorSetWriteInfo::DescriptorData{
                        .m_textureLayout = TextureLayout::ShaderResource,
                        .m_handle = _gBufferAlbedo.m_handle,
                    } }
                },
                {
                    .m_index = indices[1],
                    .m_descriptorData = { DescriptorSetWriteInfo::DescriptorData{
                        .m_textureLayout = TextureLayout::ShaderResource,
                        .m_handle = _gBufferNormal.m_handle,
                    } }
                },
                {
                    .m_index = indices[2],
                    .m_descriptorData = { DescriptorSetWriteInfo::DescriptorData{
                        .m_textureLayout = TextureLayout::ShaderResource,
                        .m_handle = _gBufferDepth.m_handle,
                    } }
                },
                {
                    .m_index = indices[3],
                    .m_descriptorData = { DescriptorSetWriteInfo::DescriptorData{
                        .m_textureLayout = TextureLayout::ShaderResource,
                        .m_handle = _gBufferAmbient.m_handle,
                    } }
                },
                {
                    .m_index = indices[4],
                    .m_descriptorData = { DescriptorSetWriteInfo::DescriptorData{
                        .m_textureLayout = TextureLayout::ShaderResource,
                        .m_handle = _deferredShadows.m_handle,
                    } }
                },
            };
            _graphicsContext->UpdateDescriptorSet(m_textureDescriptors, { writeInfo });
        }

        // Create PSO layout
        {
            PipelineLayoutDesc layoutDesc = {
                .m_descriptorSets = {
                    _sceneConstantsDescriptorSetLayout,
                    m_texturesDescriptorSetLayout
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

    void DeferredShadingPass::Render(
        const Modules::RenderGraph::RenderGraph& _,
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

    void DeferredShadingPass::CreatePso(GraphicsContext* _graphicsContext, RenderPassHandle _renderPass)
    {
        if (m_pso != GenPool::kInvalidHandle)
            return;

        m_pso = FullscreenPassCommon::CreatePso(
            _graphicsContext,
            m_allocator,
            _renderPass,
            m_pipelineLayout,
            "Shaders/DeferredShading_DeferredShadingMain",
            "DeferredShadingMain",
            false);
    }
}