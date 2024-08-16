/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include <Window/Input/InputEnums.hpp>
#include <Window/Input/InputPhysicalKeys.hpp>

namespace KryneEngine::GLFW
{
    [[nodiscard]] InputPhysicalKeys ToInputPhysicalKeys(s32 _glfwKey);
    [[nodiscard]] InputActionType ToInputEventAction(s32 _glfwAction);
    [[nodiscard]] KeyInputModifiers ToInputEventModifiers(s32 _glfwMods);
    [[nodiscard]] MouseInputButton ToMouseInputButton(s32 _glfwMouse);
}
