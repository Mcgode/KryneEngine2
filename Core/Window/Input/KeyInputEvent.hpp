/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include "InputPhysicalKeys.hpp"

#include <Common/BitUtils.hpp>

namespace KryneEngine
{
    struct KeyInputEvent
    {
        enum class Action: u8
        {
            Unknown,
            StartPress,
            KeepPressing,
            StopPress,
        };

        enum class Modifiers: u8
        {
            None        = 0,
            Ctrl        = 1 << 0,
            Shift       = 1 << 1,
            Alt         = 1 << 2,
            Super       = 1 << 3,
            CapsLock    = 1 << 4,
            NumLock     = 1 << 5,
        };

        InputPhysicalKeys m_physicalKey;
        s32 m_customCode;
        Action m_action;
        Modifiers m_modifiers;
    };

    KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(KeyInputEvent::Modifiers)
}
