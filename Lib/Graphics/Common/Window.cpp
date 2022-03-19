/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "Window.hpp"

namespace KryneEngine
{
    Window::Window(const KryneEngine::Window::Params &_params)
    {
        glfwInit();

#if defined(KE_GRAPHICS_API_VK)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
        glfwWindowHint(GLFW_RESIZABLE, _params.m_resizable);

        m_glfwWindow = glfwCreateWindow(_params.m_width, _params.m_height, _params.m_windowName.c_str(), nullptr, nullptr);
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

