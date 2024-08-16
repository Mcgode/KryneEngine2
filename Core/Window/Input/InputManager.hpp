/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include "KeyInputEvent.hpp"
#include <EASTL/functional.h>
#include <EASTL/vector_map.h>

struct GLFWwindow;

namespace KryneEngine
{
    class Window;

    class InputManager
    {
    public:
        explicit InputManager(Window* _window);

        [[nodiscard]] u32 RegisterKeyInputEventCallback(eastl::function<void(const KeyInputEvent&)>&& _callback);

    protected:
        static void KeyCallback(GLFWwindow* _window, s32 _key, s32 _scancode, s32 _action, s32 _mods);

        eastl::vector_map<u32, eastl::function<void(const KeyInputEvent&)>> m_keyInputEventListeners;
        u32 m_keyInputEventCounter { 0 };
    };
} // namespace KryneEngine