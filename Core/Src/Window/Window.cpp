/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "KryneEngine/Core/Window/Window.hpp"

#include "KryneEngine/Core/Graphics/GraphicsContext.hpp"
#include "KryneEngine/Core/Profiling/TracyHeader.hpp"
#include "KryneEngine/Core/Window/Input/InputManager.hpp"

namespace KryneEngine
{
    Window::Window(const GraphicsCommon::ApplicationInfo &_appInfo, AllocatorInstance _allocator)
        : m_allocator(_allocator)
        , m_windowFocusEventListeners(_allocator)
    {
        KE_ZoneScopedFunction("Window init");

        {
            KE_ZoneScoped("GLFW init");
            glfwInit();
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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

        m_graphicsContext = GraphicsContext::Create(_appInfo, this, _allocator);

        {
            KE_ZoneScoped("Input management init");

            m_inputManager = m_allocator.New<InputManager>(this, _allocator);

            glfwSetWindowFocusCallback(m_glfwWindow, WindowFocusCallback);
        }
    }

    Window::~Window()
    {
        m_allocator.Delete(m_inputManager);
        GraphicsContext::Destroy(m_graphicsContext);

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

