/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#include "Context.hpp"
#include <Common/Utils/Alignment.hpp>
#include <Graphics/Common/Drawing.hpp>
#include <Graphics/Common/ResourceViews/ShaderResourceView.hpp>
#include <Graphics/Common/ShaderPipeline.hpp>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/Common/Window.hpp>
#include <fstream>
#include <imgui_internal.h>

namespace KryneEngine::Modules::ImGui
{
    struct VertexEntry
    {
        float2 m_position;
        float2 m_uv;
        u32 m_color;
    };

    struct PushConstants
    {
        float2 m_scale;
        float2 m_translate;
    };

    Context::Context(GraphicsContext& _graphicsContext, RenderPassHandle _renderPass)
    {
        m_context = ::ImGui::CreateContext();

        ImGuiIO& io = ::ImGui::GetIO();
        io.BackendRendererUserData = nullptr;
        io.BackendRendererName = "KryneEngineGraphics";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

        {
            const BufferCreateDesc bufferCreateDesc{
                .m_desc = {
                    .m_size = kInitialSize * sizeof(VertexEntry),
#if !defined(KE_FINAL)
                    .m_debugName = "ImGuiContext/DynamicVertexBuffer"
#endif
                },
                .m_usage = MemoryUsage::StageEveryFrame_UsageType
                           | MemoryUsage::VertexBuffer
                           | MemoryUsage::TransferDstBuffer,
            };
            m_dynamicVertexBuffer.Init(
                _graphicsContext,
                bufferCreateDesc,
                _graphicsContext.GetFrameContextCount());
        }

        {
            const BufferCreateDesc bufferCreateDesc{
                .m_desc = {
                    .m_size = kInitialSize * sizeof(u32),
#if !defined(KE_FINAL)
                    .m_debugName = "ImGuiContext/DynamicIndexBuffer"
#endif
                },
                .m_usage = MemoryUsage::StageEveryFrame_UsageType
                           | MemoryUsage::IndexBuffer
                           | MemoryUsage::TransferDstBuffer,
            };
            m_dynamicIndexBuffer.Init(
                _graphicsContext,
                bufferCreateDesc,
                _graphicsContext.GetFrameContextCount());
        }

        _InitPso(_graphicsContext, _renderPass);
    }

    Context::~Context() { KE_ASSERT_MSG(m_context == nullptr, "ImGui module was not shut down"); }

    void Context::Shutdown(GraphicsContext& _graphicsContext)
    {
        m_dynamicIndexBuffer.Destroy(_graphicsContext);
        m_dynamicVertexBuffer.Destroy(_graphicsContext);

        if (m_fontTextureSrvHandle != GenPool::kInvalidHandle)
        {
            _graphicsContext.DestroyTextureSrv(m_fontTextureSrvHandle);
        }

        if (m_fontsTextureHandle != GenPool::kInvalidHandle)
        {
            _graphicsContext.DestroyTexture(m_fontsTextureHandle);
        }

        if (m_fontsStagingHandle != GenPool::kInvalidHandle)
        {
            _graphicsContext.DestroyBuffer(m_fontsStagingHandle);
        }

        ::ImGui::DestroyContext(m_context);
        m_context = nullptr;
    }

    void Context::NewFrame(GraphicsContext& _graphicsContext, CommandList _commandList)
    {
        ::ImGui::SetCurrentContext(m_context);

        ImGuiIO& io = ::ImGui::GetIO();

        {
            auto* window = _graphicsContext.GetWindow()->GetGlfwWindow();

            int x, y;
            glfwGetWindowSize(window, &x, &y);
            io.DisplaySize = ImVec2(float(x), float(y));

            if (x > 0 && y > 0)
            {
                int displayW, displayH;
                glfwGetFramebufferSize(window, &displayW, &displayH);
                io.DisplayFramebufferScale =
                    ImVec2(float(displayW) / io.DisplaySize.x, float(displayH) / io.DisplaySize.y);
            }
        }

        if (m_fontsTextureHandle == GenPool::kInvalidHandle)
        {
            u8* data;
            s32 w, h;
            io.Fonts->GetTexDataAsAlpha8(&data, &w, &h);

            TextureDesc fontsTextureDesc {
                .m_dimensions = {w, h, 1},
                .m_format = TextureFormat::R8_UNorm,
                .m_arraySize = 1,
                .m_type = TextureTypes::Single2D,
                .m_mipCount = 1,
#if !defined(KE_FINAL)
                .m_debugName = "ImGui/FontTexture"
#endif
            };

            const TextureCreateDesc textureCreateDesc {
                .m_desc = fontsTextureDesc,
                .m_footprintPerSubResource = _graphicsContext.FetchTextureSubResourcesMemoryFootprints(fontsTextureDesc),
                .m_memoryUsage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::TransferDstImage | MemoryUsage::SampledImage,
            };

            m_stagingFrame = _graphicsContext.GetFrameId();
            m_fontsStagingHandle = _graphicsContext.CreateStagingBuffer(fontsTextureDesc, textureCreateDesc.m_footprintPerSubResource);
            m_fontsTextureHandle = _graphicsContext.CreateTexture(textureCreateDesc);

            {
                // Set up font srv
                const TextureSrvDesc srvDesc {
                    .m_texture = m_fontsTextureHandle,
                    .m_componentsMapping = {
                        TextureComponentMapping::Red,
                        TextureComponentMapping::Red,
                        TextureComponentMapping::Red,
                        TextureComponentMapping::Red,
                    },
                    .m_format = textureCreateDesc.m_desc.m_format,
                };
                m_fontTextureSrvHandle = _graphicsContext.CreateTextureSrv(srvDesc);

                // Set up font sampler
                const SamplerDesc samplerDesc {}; // Default sampler works great for us
                m_fontSamplerHandle = _graphicsContext.CreateSampler(samplerDesc);

                // Set font descriptor set values
                DescriptorSetWriteInfo writeInfo[2] = {
                    {
                        .m_index = m_setIndices[0],
                        .m_handles = { m_fontTextureSrvHandle.m_handle },
                        .m_textureLayout = TextureLayout::ShaderResource,
                    },
                    {
                        .m_index = m_setIndices[1],
                        .m_handles = { m_fontSamplerHandle.m_handle },
                    }
                };
                _graphicsContext.UpdateDescriptorSet(
                    m_fontDescriptorSet,
                    writeInfo);
            }

            io.Fonts->SetTexID(reinterpret_cast<void*>(static_cast<u32>(m_fontTextureSrvHandle.m_handle)));

            {
                BufferMemoryBarrier stagingBufferBarrier {
                    .m_stagesSrc = BarrierSyncStageFlags::None,
                    .m_stagesDst = BarrierSyncStageFlags::Transfer,
                    .m_accessSrc = BarrierAccessFlags::None,
                    .m_accessDst = BarrierAccessFlags::TransferSrc,
                    .m_offset = 0,
                    .m_size = eastl::numeric_limits<u64>::max(),
                    .m_buffer = m_fontsStagingHandle,
                };

                TextureMemoryBarrier textureMemoryBarrier {
                    .m_stagesSrc = BarrierSyncStageFlags::None,
                    .m_stagesDst = BarrierSyncStageFlags::Transfer,
                    .m_accessSrc = BarrierAccessFlags::None,
                    .m_accessDst = BarrierAccessFlags::TransferDst,
                    .m_texture = m_fontsTextureHandle,
                    .m_layoutSrc = TextureLayout::Unknown,
                    .m_layoutDst = TextureLayout::TransferDst,
                };

                _graphicsContext.PlaceMemoryBarriers(
                    _commandList,
                    {},
                    { &stagingBufferBarrier, 1 },
                    { &textureMemoryBarrier, 1 });
            }

            _graphicsContext.SetTextureData(
                _commandList,
                m_fontsStagingHandle,
                m_fontsTextureHandle,
                textureCreateDesc.m_footprintPerSubResource[0],
                { textureCreateDesc.m_desc, 0 },
                data);

            {
                // Don't care about staging buffer state any more, they've outlived their usefulness

                TextureMemoryBarrier textureMemoryBarrier {
                    .m_stagesSrc = BarrierSyncStageFlags::Transfer,
                    .m_stagesDst = BarrierSyncStageFlags::FragmentShading,
                    .m_accessSrc = BarrierAccessFlags::TransferDst,
                    .m_accessDst = BarrierAccessFlags::ShaderResource,
                    .m_texture = m_fontsTextureHandle,
                    .m_layoutSrc = TextureLayout::TransferDst,
                    .m_layoutDst = TextureLayout::ShaderResource,
                };

                _graphicsContext.PlaceMemoryBarriers(
                    _commandList,
                    {},
                    {},
                    { &textureMemoryBarrier, 1 });
            }
        }

        if (m_fontsStagingHandle != GenPool::kInvalidHandle && _graphicsContext.IsFrameExecuted(m_stagingFrame))
        {
            _graphicsContext.DestroyBuffer(m_fontsStagingHandle);
            m_fontsStagingHandle = GenPool::kInvalidHandle;
        }

        ::ImGui::NewFrame();
    }

    void Context::PrepareToRenderFrame(GraphicsContext& _graphicsContext, CommandList _commandList)
    {
        ::ImGui::Render();

        ImDrawData* drawData = ::ImGui::GetDrawData();

        const u8 frameIndex = _graphicsContext.GetCurrentFrameContextIndex();

        {
            const u64 vertexCount = drawData->TotalVtxCount;

            const u64 desiredSize = sizeof(VertexEntry) * Alignment::NextPowerOfTwo(vertexCount);
            if (m_dynamicVertexBuffer.GetSize(frameIndex) < desiredSize)
            {
                m_dynamicVertexBuffer.RequestResize(desiredSize);
            }

            auto* vertexEntries = static_cast<VertexEntry*>(m_dynamicVertexBuffer.Map(_graphicsContext, frameIndex));
            u64 vertexIndex = 0;
            for (auto i = 0u; i < drawData->CmdListsCount; i++)
            {
                const ImDrawList* drawList = drawData->CmdLists[i];
                for (auto j = 0; j < drawList->VtxBuffer.Size; j++)
                {
                    VertexEntry& entry = vertexEntries[vertexIndex];
                    const ImDrawVert& vert = drawList->VtxBuffer[j];

                    entry.m_position = { vert.pos.x, vert.pos.y };
                    entry.m_uv = { vert.uv.x, vert.uv.y };
                    entry.m_color = vert.col;

                    vertexIndex++;
                }
            }
            m_dynamicVertexBuffer.Unmap(_graphicsContext);

            m_dynamicVertexBuffer.PrepareBuffers(
                _graphicsContext,
                _commandList,
                BarrierAccessFlags::VertexBuffer,
                frameIndex);
        }

        {
            const u64 indexCount = drawData->TotalIdxCount;

            const u64 desiredSize = sizeof(u32) * Alignment::NextPowerOfTwo(indexCount);
            if (m_dynamicIndexBuffer.GetSize(frameIndex) < desiredSize)
            {
                m_dynamicIndexBuffer.RequestResize(desiredSize);
            }

            u32* indexBuffer = static_cast<u32*>(m_dynamicIndexBuffer.Map(_graphicsContext, frameIndex));
            u32 offset = 0;
            for (auto i = 0u; i < drawData->CmdListsCount; i++)
            {
                const ImDrawList* drawList = drawData->CmdLists[i];
                for (auto j = 0; j < drawList->IdxBuffer.Size; j++)
                {
                    indexBuffer[j] = drawList->IdxBuffer[j] + offset;
                }
                indexBuffer += drawList->IdxBuffer.Size;
                offset += drawList->VtxBuffer.Size;
            }
            m_dynamicIndexBuffer.Unmap(_graphicsContext);

            m_dynamicIndexBuffer.PrepareBuffers(
                _graphicsContext,
                _commandList,
                BarrierAccessFlags::IndexBuffer,
                frameIndex);
        }
    }

    void Context::RenderFrame(GraphicsContext& _graphicsContext, CommandList _commandList)
    {
        ImDrawData* drawData = ::ImGui::GetDrawData();

        // Set viewport
        {
            const Viewport viewport {
                .m_width = static_cast<s32>(drawData->DisplaySize.x),
                .m_height = static_cast<s32>(drawData->DisplaySize.y),
            };
            _graphicsContext.SetViewport(_commandList, viewport);
        }

        const u8 frameIndex = _graphicsContext.GetCurrentFrameContextIndex();

        // Set index buffer
        {
            _graphicsContext.SetIndexBuffer(
                _commandList,
                m_dynamicIndexBuffer.GetBuffer(frameIndex),
                m_dynamicIndexBuffer.GetSize(frameIndex),
                false);
        }

        // Set vertex buffer
        {
            BufferView bufferView {
                .m_size = m_dynamicVertexBuffer.GetSize(frameIndex),
                .m_stride = sizeof(VertexEntry),
                .m_buffer = m_dynamicVertexBuffer.GetBuffer(frameIndex),
            };
            _graphicsContext.SetVertexBuffers(_commandList, {&bufferView,1});
        }

        u64 vertexOffset = 0;
        u64 indexOffset = 0;

        for (auto i = 0u; i < drawData->CmdListsCount; i++)
        {
            const ImDrawList* drawList = drawData->CmdLists[i];

            for (const auto& drawCmd : drawList->CmdBuffer)
            {
                // If user callback, run it instead
                if (drawCmd.UserCallback != nullptr)
                {
                    drawCmd.UserCallback(drawList, &drawCmd);
                    continue;
                }

                // Set up scissor rect
                {
                    const ImVec2 clipOffset = drawData->DisplayPos;
                    const ImVec2 clipMin{drawCmd.ClipRect.x - clipOffset.x, drawCmd.ClipRect.y - clipOffset.y};
                    const ImVec2 clipMax{drawCmd.ClipRect.z - clipOffset.x, drawCmd.ClipRect.w - clipOffset.y};

                    const Rect rect{
                        .m_left = static_cast<u32>(clipMin.x),
                        .m_top = static_cast<u32>(clipMin.y),
                        .m_right = static_cast<u32>(clipMax.x),
                        .m_bottom = static_cast<u32>(clipMax.y),
                    };
                    _graphicsContext.SetScissorsRect(_commandList, rect);
                }

                // Draw
                {
                    _graphicsContext.SetGraphicsPipeline(_commandList, m_pso);

                    _graphicsContext.SetGraphicsDescriptorSets(_commandList, { &m_fontDescriptorSet, 1 });

                    PushConstants pushConstants {};
                    pushConstants.m_scale = {
                        2.0f / drawData->DisplaySize.x,
                        -2.0f / drawData->DisplaySize.y
                    };
                    pushConstants.m_translate = {
                        -1.0f - drawData->DisplayPos.x * pushConstants.m_scale.x,
                        1.0f - drawData->DisplayPos.y * pushConstants.m_scale.y,
                    };
                    _graphicsContext.SetGraphicsPushConstant(
                        _commandList,
                        2,
                        { reinterpret_cast<u32*>(&pushConstants), 4 });

                    const DrawIndexedInstancedDesc desc {
                        .m_elementCount = drawCmd.ElemCount,
                        .m_indexOffset = static_cast<u32>(indexOffset + drawCmd.IdxOffset),
                        .m_vertexOffset = static_cast<u32>(vertexOffset + drawCmd.VtxOffset),
                    };
                    _graphicsContext.DrawIndexedInstanced(_commandList, desc);
                }
            }

            vertexOffset += drawList->VtxBuffer.Size;
            indexOffset += drawList->IdxBuffer.Size;
        }
    }

    void Context::_InitPso(GraphicsContext& _graphicsContext, RenderPassHandle _renderPass)
    {
        // Read shader files
        {
            constexpr auto readShaderFile = [](const char* _path, auto& _vec)
            {
                std::ifstream file(_path, std::ios::binary);
                VERIFY_OR_RETURN_VOID(file);

                file.seekg(0, std::ios::end);
                _vec.resize(file.tellg());
                file.seekg(0, std::ios::beg);

                KE_VERIFY(file.read(_vec.data(), _vec.size()));
            };

            readShaderFile("Shaders/ImGui/ImGui_vs_MainVS.cso", m_vsBytecode);
            readShaderFile("Shaders/ImGui/ImGui_ps_MainPS.cso", m_fsBytecode);

            m_vsModule = _graphicsContext.RegisterShaderModule(m_vsBytecode.data(), m_vsBytecode.size());
            m_fsModule = _graphicsContext.RegisterShaderModule(m_fsBytecode.data(), m_fsBytecode.size());
        }

        // Set up descriptor set
        {
            DescriptorSetDesc desc {
                .m_bindings = {
                    {
                        .m_type = DescriptorBindingDesc::Type::SampledTexture,
                        .m_visibility = ShaderVisibility::Fragment,
                    },
                    {
                        .m_type = DescriptorBindingDesc::Type::Sampler,
                        .m_visibility = ShaderVisibility::Fragment,
                    },
                }
            };
            m_setIndices.resize(desc.m_bindings.size());
            m_fontDescriptorSet = _graphicsContext.CreateDescriptorSet(desc, m_setIndices.data());
        }

        // Pipeline layout creation
        {
            PipelineLayoutDesc pipelineLayoutDesc {};

            pipelineLayoutDesc.m_descriptorSets.push_back(m_fontDescriptorSet);

            // Scale and translate push constant
            pipelineLayoutDesc.m_pushConstants.push_back(PushConstantDesc {
                .m_sizeInBytes = sizeof(float2) + sizeof(float2),
            });

            m_pipelineLayout = _graphicsContext.CreatePipelineLayout(pipelineLayoutDesc);
        }

        // PSO creation
        {
            GraphicsPipelineDesc desc {
                .m_rasterState = {
                    .m_cullMode = RasterStateDesc::CullMode::None,
                    .m_depthClip = false,
                },
                .m_colorBlending = {
                    .m_attachments = { { kDefaultColorAttachmentAlphaBlendDesc } },
                },
                .m_depthStencil = {
                    .m_depthTest = false,
                    .m_depthWrite = false,
                    .m_depthCompare = DepthStencilStateDesc::CompareOp::Always,
                },
                .m_renderPass = _renderPass,
                .m_pipelineLayout = m_pipelineLayout,
#if !defined(KE_FINAL)
                .m_debugName = "ImGui_Render_PSO",
#endif
            };

            desc.m_stages.push_back(GraphicsShaderStage {
                .m_shaderModule = m_vsModule,
                .m_stage = GraphicsShaderStage::Stage::Vertex,
            });
            desc.m_stages.push_back(GraphicsShaderStage {
                .m_shaderModule = m_fsModule,
                .m_stage = GraphicsShaderStage::Stage::Fragment,
            });

            desc.m_vertexLayout = {
                VertexLayoutElement {
                    .m_semanticName = VertexLayoutElement::SemanticName::Position,
                    .m_semanticIndex = 0,
                    .m_bindingIndex = 0,
                    .m_format = TextureFormat::RG32_Float,
                    .m_offset = offsetof(VertexEntry, m_position),
                },
                VertexLayoutElement {
                    .m_semanticName = VertexLayoutElement::SemanticName::Uv,
                    .m_semanticIndex = 0,
                    .m_bindingIndex = 0,
                    .m_format = TextureFormat::RG32_Float,
                    .m_offset = offsetof(VertexEntry, m_uv),
                },
                VertexLayoutElement {
                    .m_semanticName = VertexLayoutElement::SemanticName::Color,
                    .m_semanticIndex = 0,
                    .m_bindingIndex = 0,
                    .m_format = TextureFormat::RGBA8_UNorm,
                    .m_offset = offsetof(VertexEntry, m_color),
                },
            };

            m_pso = _graphicsContext.CreateGraphicsPipeline(desc);
        }
    }
} // namespace KryneEngine