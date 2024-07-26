/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#include "ImGuiModule.hpp"
#include <Graphics/Common/Texture.hpp>
#include <Graphics/Common/GraphicsContext.hpp>
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
        ImGui::DestroyContext(m_context);
        m_context = nullptr;
    }

    void ImGuiModule::NewFrame(GraphicsContext& _graphicsContext)
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

            TextureDesc imageDesc {
                .m_dimensions = { w, h, 1 },
                .m_format = TextureFormat::R8_UNorm,
                .m_arraySize = 1,
                .m_type = TextureTypes::Single2D,
                .m_mipCount = 1,
#if !defined(KE_FINAL)
                .m_debugName = "ImGui/FontImage"
#endif
            };

            m_fontsTextureHandle = GenPool::kInvalidHandle;
            io.Fonts->SetTexID(reinterpret_cast<void*>(static_cast<u32>(m_fontsTextureHandle)));
        }

        ImGui::NewFrame();
    }

    void ImGuiModule::RenderFrame(GraphicsContext& _graphicsContext)
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