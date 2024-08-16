/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include "InputEnums.hpp"
#include "InputPhysicalKeys.hpp"

namespace KryneEngine
{
    struct KeyInputEvent
    {
        InputPhysicalKeys m_physicalKey;
        s32 m_customCode;
        InputActionType m_action;
        KeyInputModifiers m_modifiers;
    };
}