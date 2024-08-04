/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#include "ImGuiModule.hpp"
#include <Graphics/Common/ResourceViews/ShaderResourceView.hpp>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/Common/Window.hpp>
#include <imgui_internal.h>

namespace KryneEngine
{
    ImGuiModule::ImGuiModule(GraphicsContext& _graphicsContext)
    {
        m_context = ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererUserData = nullptr;
        io.BackendRendererName = "KryneEngineGraphics";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    }

    ImGuiModule::~ImGuiModule() { KE_ASSERT_MSG(m_context == nullptr, "ImGui module was not shut down"); }

    void ImGuiModule::Shutdown(GraphicsContext& _graphicsContext)
    {
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

        ImGui::DestroyContext(m_context);
        m_context = nullptr;
    }

    void ImGuiModule::NewFrame(GraphicsContext& _graphicsContext, CommandList _commandList)
    {
        ImGui::SetCurrentContext(m_context);

        ImGuiIO& io = ImGui::GetIO();

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

        ImGui::NewFrame();
    }

    void ImGuiModule::PrepareToRenderFrame(GraphicsContext& _graphicsContext, CommandList _commandList)
    {
        ImGui::Render();

        ImGuiContext& context = *m_context;

        ImDrawData* drawData = ImGui::GetDrawData();

        u64 vertexCount = drawData->TotalVtxCount;
        u64 indexCount = drawData->TotalIdxCount;
    }

    void ImGuiModule::RenderFrame(GraphicsContext& _graphicsContext, CommandList _commandList)
    {

    }
} // namespace KryneEngine