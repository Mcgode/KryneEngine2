/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include <imgui.h>
#include <Window/Input/InputEnums.hpp>
#include <Window/Input/InputPhysicalKeys.hpp>

namespace KryneEngine
{
    class Window;
}

namespace KryneEngine::Modules::ImGui
{
    class Input
    {
    public:
        explicit Input(Window* _window);

        void Shutdown(Window* _window);

    private:
        u32 m_keyCallbackId;
        u32 m_cursorPosCallbackId;
        u32 m_mouseBtnCallbackId;
        u32 m_scrollEventCallbackId;

        static void ApplyModifiers(KeyInputModifiers _modifiers);

        [[nodiscard]] static ImGuiKey ToImGuiKey(InputPhysicalKeys _key);
        [[nodiscard]] static ImGuiMouseButton ToImGuiMouseButton(MouseInputButton _mouseButton);
    };
} // namespace KryneEngine