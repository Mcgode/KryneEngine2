/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include <imgui.h>
#include <Window/Input/InputEnums.hpp>
#include <Window/Input/InputPhysicalKeys.hpp>

namespace KryneEngine::Modules::ImGui
{
    namespace InputConverters
    {
        [[nodiscard]] ImGuiKey ToImGuiKey(InputPhysicalKeys _key);
        [[nodiscard]] ImGuiMouseButton ToImGuiMouseButton(MouseInputButton _mouseButton);
    }
} // namespace KryneEngine