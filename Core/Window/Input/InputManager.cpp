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
        glfwSetMouseButtonCallback(glfwWindow, MouseButtonInputCallback);
        glfwSetScrollCallback(glfwWindow, ScrollCallback);
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

    u32 InputManager::RegisterCursorPosEventCallback(eastl::function<void(float, float)>&& _callback)
    {
        const u32 id = m_cursorPosEventCounter++;
        m_cursorPosEventListeners.emplace(id, _callback);
        return id;
    }

    void InputManager::UnregisterCursorPosEventCallback(u32 _id)
    {
        m_cursorPosEventListeners.erase(_id);
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

    u32 InputManager::RegisterScrollInputEventCallback(eastl::function<void(float, float)>&& _callback)
    {
        const u32 id = m_scrollInputEventCounter++;
        m_scrollInputEventListeners.emplace(id, _callback);
        return id;
    }

    void InputManager::UnregisterScrollInputEventCallback(u32 _id)
    {
        m_scrollInputEventListeners.erase(_id);
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

        for (const auto& pair : inputManager->m_cursorPosEventListeners)
        {
            pair.second(static_cast<float>(_posX), static_cast<float>(_posY));
        }
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

    void InputManager::ScrollCallback(GLFWwindow* _window, double _xScroll, double _yScroll)
    {
        auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(_window));

        for (const auto& pair : inputManager->m_scrollInputEventListeners)
        {
            pair.second(static_cast<float>(_xScroll), static_cast<float>(_yScroll));
        }
    }
} // namespace KryneEngine