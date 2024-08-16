/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include <Common/BitUtils.hpp>

namespace KryneEngine
{
    enum class InputActionType: u8
    {
        Unknown,
        StartPress,
        KeepPressing,
        StopPress,
    };

    enum class KeyInputModifiers : u8
    {
        None        = 0,
        Ctrl        = 1 << 0,
        Shift       = 1 << 1,
        Alt         = 1 << 2,
        Super       = 1 << 3,
        CapsLock    = 1 << 4,
        NumLock     = 1 << 5,
    };
    KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(KeyInputModifiers)

    enum class MouseInputButton : u8
    {
        Unknown,

        Left,
        Right,
        Middle,

        Button1 = Left,
        Button2 = Right,
        Button3 = Middle,
        Button4,
        Button5,
        Button6,
        Button7,
        Button8,
        Button9,
        Button10,
    };
}
