/**
 * @file
 * @author Max Godefroy
 * @date 16/08/2024.
 */

#include "InputManager.hpp"

#include <Window/GLFW/Input/KeyInputEvent.hpp>
#include <Window/Window.hpp>

namespace KryneEngine
{
    InputManager::InputManager(Window* _window)
    {
        GLFWwindow* glfwWindow = _window->GetGlfwWindow();

        glfwSetWindowUserPointer(glfwWindow, this);

        glfwSetKeyCallback(glfwWindow, KeyCallback);
        glfwSetCursorPosCallback(glfwWindow, CursorPosCallback);
    }

    u32 InputManager::RegisterKeyInputEventCallback(eastl::function<void(const KeyInputEvent&)>&& _callback)
    {
        const u32 id = m_keyInputEventCounter++;
        m_keyInputEventListeners.emplace(id, _callback);
        return id;
    }

    void InputManager::UnregisterKeyInputEventCallback(u32 _id)
    {
        m_keyInputEventListeners.erase(_id);
    }

    u32 InputManager::RegisterMouseInputEventCallback(eastl::function<void(const MouseInputEvent&)>&& _callback)
    {
        const u32 id = m_mouseInputEventCounter++;
        m_mouseInputEventListeners.emplace(id, _callback);
        return id;
    }

    void InputManager::UnregisterMouseInputEventCallback(u32 _id)
    {
        m_mouseInputEventListeners.erase(_id);
    }

    void InputManager::KeyCallback(GLFWwindow* _window, s32 _key, s32 _scancode, s32 _action, s32 _mods)
    {
        auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(_window));

        const KeyInputEvent keyInputEvent {
            .m_physicalKey = GLFW::ToInputPhysicalKeys(_key),
            .m_customCode = _scancode,
            .m_action = GLFW::ToInputEventAction(_action),
            .m_modifiers = GLFW::ToInputEventModifiers(_mods),
        };

        for (const auto& pair : inputManager->m_keyInputEventListeners)
        {
            pair.second(keyInputEvent);
        }
    }

    void InputManager::CursorPosCallback(GLFWwindow* _window, double _posX, double _posY)
    {
        auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(_window));
        inputManager->m_cursorPos = {
            _posX,
            _posY
        };
    }

    void InputManager::MouseButtonInputCallback(GLFWwindow* _window, s32 _button, s32 _action, s32 _mods)
    {
        auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(_window));

        const MouseInputEvent mouseInputEvent{
            .m_mouseButton = GLFW::ToMouseInputButton(_button),
            .m_action = GLFW::ToInputEventAction(_action),
            .m_modifiers = GLFW::ToInputEventModifiers(_mods),
        };

        for (const auto& pair : inputManager->m_mouseInputEventListeners)
        {
            pair.second(mouseInputEvent);
        }
    }
} // namespace KryneEngine