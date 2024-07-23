/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#include "ImGuiModule.hpp"
#include <imgui_internal.h>
#include <Graphics/Common/GraphicsContext.hpp>
#include <Graphics/Common/Window.hpp>

namespace KryneEngine
{
    ImGuiModule::ImGuiModule(GraphicsContext &_graphicsContext)
    {
        m_context = ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererUserData = nullptr;
        io.BackendRendererName = "KryneEngineGraphics";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    }

    ImGuiModule::~ImGuiModule()
    {
        KE_ASSERT_MSG(m_context == nullptr, "ImGui module was not shut down");
    }

    void ImGuiModule::Shutdown(GraphicsContext &_graphicsContext)
    {
        ImGui::DestroyContext(m_context);
        m_context = nullptr;
    }

    void ImGuiModule::NewFrame(GraphicsContext &_graphicsContext)
    {
        ImGui::SetCurrentContext(m_context);

        ImGuiIO& io = ImGui::GetIO();

        {
            auto *window = _graphicsContext.GetWindow()->GetGlfwWindow();

            int x, y;
            glfwGetWindowSize(window, &x, &y);
            io.DisplaySize = ImVec2(float(x), float(y));

            if (x > 0 && y > 0)
            {
                int displayW, displayH;
                glfwGetFramebufferSize(window, &displayW, &displayH);
                io.DisplayFramebufferScale = ImVec2(float(displayW) / io.DisplaySize.x, float(displayH) / io.DisplaySize.y);
            }
        }

        ImGui::NewFrame();
    }

    void ImGuiModule::RenderFrame(GraphicsContext &_graphicsContext)
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
}// namespace KryneEngine