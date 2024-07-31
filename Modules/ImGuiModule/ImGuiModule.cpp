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
            // TODO: Destroy Srv
        }

        if (m_fontsTextureHandle != GenPool::kInvalidHandle)
        {
            _graphicsContext.DestroyTexture(m_fontsTextureHandle);
        }

        if (m_fontsStagingHandle != GenPool::kInvalidHandle)
        {
            _graphicsContext.DestroyTexture(m_fontsStagingHandle);
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

            TextureCreateDesc stagingTextureCreateDesc{
                .m_desc = {
                    .m_dimensions = {w, h, 1},
                    .m_format = TextureFormat::R8_UNorm,
                    .m_arraySize = 1,
                    .m_type = TextureTypes::Single2D,
                    .m_mipCount = 1,
#if !defined(KE_FINAL)
                    .m_debugName = "ImGui/FontTexture"
#endif
                },
                .m_memoryUsage = MemoryUsage::StageOnce_UsageType | MemoryUsage::TransferSrcImage,
            };
            stagingTextureCreateDesc.m_footprintPerSubResource =
                _graphicsContext.FetchTextureSubResourcesMemoryFootprints(stagingTextureCreateDesc.m_desc);

            TextureCreateDesc textureCreateDesc{
                .m_desc = stagingTextureCreateDesc.m_desc,
                .m_footprintPerSubResource = stagingTextureCreateDesc.m_footprintPerSubResource,
                .m_memoryUsage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::TransferDstImage | MemoryUsage::SampledImage,
            };

            m_stagingFrame = _graphicsContext.GetFrameId();
            m_fontsStagingHandle = _graphicsContext.CreateTexture(stagingTextureCreateDesc);
            m_fontsTextureHandle = _graphicsContext.CreateTexture(textureCreateDesc);

            {
                TextureSrvDesc srvDesc {
                    .m_textureHandle = m_fontsTextureHandle,
                    .m_format = textureCreateDesc.m_desc.m_format,
                };
                m_fontsTextureSrvHandle = _graphicsContext.CreateTextureSrv(srvDesc);
            }

            io.Fonts->SetTexID(reinterpret_cast<void*>(static_cast<u32>(m_fontsTextureSrvHandle)));

            _graphicsContext.SetTextureData(
                _commandList,
                m_fontsStagingHandle,
                m_fontsTextureHandle,
                stagingTextureCreateDesc.m_footprintPerSubResource[0],
                0,
                data);
        }

        if (m_fontsStagingHandle != GenPool::kInvalidHandle && _graphicsContext.IsFrameExecuted(m_stagingFrame))
        {
            _graphicsContext.DestroyTexture(m_fontsStagingHandle);
            m_fontsStagingHandle = GenPool::kInvalidHandle;
        }

        ImGui::NewFrame();
    }

    void ImGuiModule::RenderFrame(GraphicsContext& _graphicsContext, CommandList _commandList)
    {
        ImGui::Render();

        ImGuiContext& context = *m_context;
        for (auto i = 0; i < context.Viewports.Size; i++)
        {
            ImGuiViewportP* viewport = context.Viewports[i];
            if (viewport == nullptr || !viewport->DrawDataP.Valid)
            {
                continue;
            }
            ImDrawData& drawData = viewport->DrawDataP;
        }
    }
} // namespace KryneEngine