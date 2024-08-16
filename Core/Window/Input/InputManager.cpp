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

    void InputManager::KeyCallback(GLFWwindow* _window, s32 _key, s32 _scancode, s32 _action, s32 _mods)
    {
        auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(_window));

        const KeyInputEvent keyInputEvent {
            .m_physicalKey = GLFW::ToInputPhysicalKeys(_key),
            .m_customCode = _scancode,
            .m_action = GLFW::ToKeyInputEventAction(_action),
            .m_modifiers = GLFW::ToKeyInputEventModifiers(_mods),
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
} // namespace KryneEngine