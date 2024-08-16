/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include <Window/Input/KeyInputEvent.hpp>

namespace KryneEngine::GLFW
{
    [[nodiscard]] InputPhysicalKeys ToInputPhysicalKeys(s32 _glfwKey);
    [[nodiscard]] KeyInputEvent::Action ToKeyInputEventAction(s32 _glfwAction);
    [[nodiscard]] KeyInputEvent::Modifiers ToKeyInputEventModifiers(s32 _glfwMods);
}
