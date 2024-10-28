/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "Window.hpp"

#include <Graphics/Common/GraphicsContext.hpp>
#include <Profiling/TracyHeader.hpp>
#include <Window/Input/InputManager.hpp>

namespace KryneEngine
{
    Window::Window(const GraphicsCommon::ApplicationInfo &_appInfo)
    {
        KE_ZoneScopedFunction("Window init");

        {
            KE_ZoneScoped("GLFW init");
            glfwInit();
        }

#if defined(KE_GRAPHICS_API_VK)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
        const auto& displayInfo = _appInfo.m_displayOptions;

        glfwWindowHint(GLFW_RESIZABLE, displayInfo.m_resizableWindow);

        {
            KE_ZoneScoped("GLFW window creation");

            m_glfwWindow = glfwCreateWindow(displayInfo.m_width,
                                            displayInfo.m_height,
                                            _appInfo.m_applicationName.c_str(),
                                            nullptr,
                                            nullptr);
        }
        glfwSetWindowUserPointer(m_glfwWindow, this);

        m_graphicsContext = eastl::make_unique<GraphicsContext>(_appInfo, this);

        {
            KE_ZoneScoped("Input management init");

            m_inputManager = eastl::make_unique<InputManager>(this);

            glfwSetWindowFocusCallback(m_glfwWindow, WindowFocusCallback);
        }
    }

    Window::~Window()
    {
        m_graphicsContext.reset();

        glfwDestroyWindow(m_glfwWindow);
        glfwTerminate();
    }

    bool Window::WaitForEvents() const
    {
        KE_ZoneScopedFunction("Window::WaitForEvents");

        glfwPollEvents();

        return !glfwWindowShouldClose(m_glfwWindow);
    }

    u32 Window::RegisterWindowFocusEventCallback(eastl::function<void(bool)>&& _callback)
    {
        const auto lock = m_callbackMutex.AutoLock();

        const u32 id = m_windowFocusEventCounter++;
        m_windowFocusEventListeners.emplace(id, _callback);
        return id;
    }

    void Window::UnregisterWindowFocusEventCallback(u32 _id)
    {
        const auto lock = m_callbackMutex.AutoLock();
        m_windowFocusEventListeners.erase(_id);
    }

    void Window::WindowFocusCallback(GLFWwindow* _window, s32 _focused)
    {
        auto* window = static_cast<Window*>(glfwGetWindowUserPointer(_window));

        const auto lock = window->m_callbackMutex.AutoLock();

        for (const auto& pair : window->m_windowFocusEventListeners)
        {
            pair.second(_focused);
        }
    }
}

