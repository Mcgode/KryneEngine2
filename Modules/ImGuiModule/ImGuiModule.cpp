/**
 * @file
 * @author Max Godefroy
 * @date 21/07/2024.
 */

#include "ImGuiModule.hpp"
#include <imgui_internal.h>

namespace KryneEngine
{
    ImGuiModule::ImGuiModule(GraphicsContext &_graphicsContext)
    {
        m_context = ImGui::CreateContext();
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
    }

    void ImGuiModule::RenderFrame(GraphicsContext &_graphicsContext)
    {
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