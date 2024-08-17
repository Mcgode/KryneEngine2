/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include "Enums.hpp"

namespace KryneEngine
{
    struct KeyInputEvent
    {
        InputKeys m_physicalKey;
        s32 m_customCode;
        InputActionType m_action;
        KeyInputModifiers m_modifiers;
    };
}
