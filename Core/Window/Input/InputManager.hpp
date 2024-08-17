/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#pragma once

#include "KeyInputEvent.hpp"
#include "MouseInputEvent.hpp"

#include <EASTL/functional.h>
#include <EASTL/vector_map.h>
#include <Threads/LightweightMutex.hpp>

struct GLFWwindow;

namespace KryneEngine
{
    class Window;

    class InputManager
    {
    public:
        explicit InputManager(Window* _window);

        [[nodiscard]] u32 RegisterKeyInputEventCallback(eastl::function<void(const KeyInputEvent&)>&& _callback);
        void UnregisterKeyInputEventCallback(u32 _id);

        [[nodiscard]] u32 RegisterTextInputEventCallback(eastl::function<void(u32)>&& _callback);
        void UnregisterTextInputEventCallback(u32 _id);

        [[nodiscard]] u32 RegisterCursorPosEventCallback(eastl::function<void(float, float)>&& _callback);
        void UnregisterCursorPosEventCallback(u32 _id);
        [[nodiscard]] const float2& GetCursorPos() const { return m_cursorPos; }

        [[nodiscard]] u32 RegisterMouseInputEventCallback(eastl::function<void(const MouseInputEvent&)>&& _callback);
        void UnregisterMouseInputEventCallback(u32 _id);

        [[nodiscard]] u32 RegisterScrollInputEventCallback(eastl::function<void(float, float)>&& _callback);
        void UnregisterScrollInputEventCallback(u32 _id);

    protected:
        LightweightMutex m_mutex;

        static void KeyCallback(GLFWwindow* _window, s32 _key, s32 _scancode, s32 _action, s32 _mods);
        eastl::vector_map<u32, eastl::function<void(const KeyInputEvent&)>> m_keyInputEventListeners;
        u32 m_keyInputEventCounter { 0 };

        static void TextCallback(GLFWwindow* _window, u32 _char);
        eastl::vector_map<u32, eastl::function<void(u32)>> m_textInputEventListeners;
        u32 m_textInputEventCounter { 0 };

        static void CursorPosCallback(GLFWwindow* _window, double _posX, double _posY);
        eastl::vector_map<u32, eastl::function<void(float, float)>> m_cursorPosEventListeners;
        u32 m_cursorPosEventCounter = 0;
        float2 m_cursorPos;

        static void MouseButtonInputCallback(GLFWwindow* _window, s32 _button, s32 _action, s32 _mods);
        eastl::vector_map<u32, eastl::function<void(const MouseInputEvent&)>> m_mouseInputEventListeners;
        u32 m_mouseInputEventCounter = 0;

        static void ScrollCallback(GLFWwindow* _window, double _xScroll, double _yScroll);
        eastl::vector_map<u32, eastl::function<void(float, float)>> m_scrollInputEventListeners;
        u32 m_scrollInputEventCounter = 0;
    };
} // namespace KryneEngine