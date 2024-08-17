/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include "Enums.hpp"

namespace KryneEngine
{
    struct MouseInputEvent
    {
        MouseInputButton m_mouseButton;
        InputActionType m_action;
        KeyInputModifiers m_modifiers;
    };
}
