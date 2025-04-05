/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2025.
 */

#include "DeferredShadingPass.hpp"

#include <fstream>
#include <KryneEngine/Core/Graphics/Common/Drawing.hpp>
#include <KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp>

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
                    DescriptorBindingDesc {}, // Albedo
                    DescriptorBindingDesc {}, // Normal
                    DescriptorBindingDesc {}, // Depth
                    DescriptorBindingDesc {}, // Ambient
                    DescriptorBindingDesc {}, // Deferred shadows
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

        const int2 resolution {
            _passExecutionData.m_graphicsContext->GetApplicationInfo().m_displayOptions.m_width,
            _passExecutionData.m_graphicsContext->GetApplicationInfo().m_displayOptions.m_height,
        };

        _passExecutionData.m_graphicsContext->SetViewport(
            _passExecutionData.m_commandList,
            { .m_width = resolution.x, .m_height = resolution.y });
        _passExecutionData.m_graphicsContext->SetScissorsRect(
            _passExecutionData.m_commandList,
            { .m_left = 0, .m_top = 0, .m_right = (u32)resolution.x, .m_bottom = (u32)resolution.y });
        _passExecutionData.m_graphicsContext->SetGraphicsPipeline(
            _passExecutionData.m_commandList,
            m_pso);
        _passExecutionData.m_graphicsContext->SetGraphicsDescriptorSets(
            _passExecutionData.m_commandList,
            m_pipelineLayout,
            { &m_sceneConstantsDescriptorSet, 2 }); // Both handles are contiguous in memory

        const float fullscreenDepth = 0.f;
        _passExecutionData.m_graphicsContext->SetGraphicsPushConstant(
            _passExecutionData.m_commandList,
            m_pipelineLayout,
            { (u32*)(&fullscreenDepth), 1 });

        _passExecutionData.m_graphicsContext->DrawInstanced(
            _passExecutionData.m_commandList,
            { .m_vertexCount = 3 });
    }

    void DeferredShadingPass::CreatePso(GraphicsContext* _graphicsContext, RenderPassHandle _renderPass)
    {
        if (m_pso != GenPool::kInvalidHandle)
            return;

        void* vsByteCode, *fsByteCode;
        const auto createShaderModule = [&](const auto& _path, void*& _data) -> ShaderModuleHandle
        {
            auto path = eastl::string(_path) + "." + _graphicsContext->GetShaderFileExtension();;

            std::ifstream file(path.c_str(), std::ios::binary);
            VERIFY_OR_RETURN(file, { GenPool::kInvalidHandle });

            file.seekg(0, std::ios::end);
            const size_t size = file.tellg();
            _data = m_allocator.allocate(size);
            file.seekg(0, std::ios::beg);

            KE_VERIFY(file.read(reinterpret_cast<char*>(_data), size));

            const ShaderModuleHandle handle = _graphicsContext->RegisterShaderModule(_data, size);
            return handle;
        };

        ShaderModuleHandle vsModule = createShaderModule("Shaders/FullScreenVS_FullScreenMain", vsByteCode);
        ShaderModuleHandle fsModule = createShaderModule("Shaders/DeferredShading_DeferredShadingMain", fsByteCode);

        GraphicsPipelineDesc psoDesc {
            .m_stages = {
                GraphicsShaderStage {
                    .m_shaderModule = vsModule,
                    .m_stage = GraphicsShaderStage::Stage::Vertex,
                    .m_entryPoint = "FullScreenMain",
                },
                GraphicsShaderStage {
                    .m_shaderModule = fsModule,
                    .m_stage = GraphicsShaderStage::Stage::Fragment,
                    .m_entryPoint = "DeferredShadingMain",
                },
            },
            .m_colorBlending = { .m_attachments = { ColorAttachmentBlendDesc() } },
            .m_depthStencil = { .m_depthTest = false, .m_depthWrite = false },
            .m_renderPass = _renderPass,
            .m_pipelineLayout = m_pipelineLayout,
        };

        m_pso = _graphicsContext->CreateGraphicsPipeline(psoDesc);

        _graphicsContext->FreeShaderModule(fsModule);
        m_allocator.deallocate(fsByteCode);
        _graphicsContext->FreeShaderModule(vsModule);
        m_allocator.deallocate(vsByteCode);
    }
}