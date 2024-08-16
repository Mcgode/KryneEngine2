/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "Window.hpp"

namespace KryneEngine
{
    Window::Window(const GraphicsCommon::ApplicationInfo &_appInfo)
    {
        glfwInit();

#if defined(KE_GRAPHICS_API_VK)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
        const auto& displayInfo = _appInfo.m_displayOptions;

        glfwWindowHint(GLFW_RESIZABLE, displayInfo.m_resizableWindow);

        m_glfwWindow = glfwCreateWindow(displayInfo.m_width,
                                        displayInfo.m_height,
                                        _appInfo.m_applicationName.c_str(),
                                        nullptr,
                                        nullptr);
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_glfwWindow);
        glfwTerminate();
    }

    bool Window::WaitForEvents() const
    {
        glfwPollEvents();

        return !glfwWindowShouldClose(m_glfwWindow);
    }
}

