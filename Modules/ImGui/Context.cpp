/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#include "Context.hpp"
#include <Common/Utils/Alignment.hpp>
#include <Graphics/Common/ResourceViews/ShaderResourceView.hpp>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/Common/Window.hpp>
#include <imgui_internal.h>

namespace KryneEngine::Modules::ImGui
{
    struct VertexEntry
    {
        float2 m_position;
        float2 m_uv;
        u32 m_color;
    };

    Context::Context(GraphicsContext& _graphicsContext)
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
    }

    Context::~Context() { KE_ASSERT_MSG(m_context == nullptr, "ImGui module was not shut down"); }

    void Context::Shutdown(GraphicsContext& _graphicsContext)
    {
        m_dynamicIndexBuffer.Destroy(_graphicsContext);
        m_dynamicVertexBuffer.Destroy(_graphicsContext);

        if (m_fontsTextureSrvHandle != GenPool::kInvalidHandle)
        {
            _graphicsContext.DestroyTextureSrv(m_fontsTextureSrvHandle);
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
                TextureSrvDesc srvDesc {
                    .m_texture = m_fontsTextureHandle,
                    .m_format = textureCreateDesc.m_desc.m_format,
                };
                m_fontsTextureSrvHandle = _graphicsContext.CreateTextureSrv(srvDesc);
            }

            io.Fonts->SetTexID(reinterpret_cast<void*>(static_cast<u32>(m_fontsTextureSrvHandle.m_handle)));

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

        ImGuiContext& context = *m_context;

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

    }
} // namespace KryneEngine