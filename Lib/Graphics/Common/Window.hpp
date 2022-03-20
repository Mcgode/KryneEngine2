/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <GLFW/glfw3.h>
#include <Common/KETypes.hpp>

namespace KryneEngine
{
    class Window
    {
    public:
        struct Params
        {
            u16 m_width = 1280;
            u16 m_height = 720;
            bool m_resizable = false;
            eastl::string m_windowName = "KryneEngine2";
        };

        explicit Window(const Params& _params);

        virtual ~Window();

        [[nodiscard]] bool WaitForEvents() const;
        [[nodiscard]] GLFWwindow* GetGlfwWindow() const { return m_glfwWindow; }

    private:
        GLFWwindow* m_glfwWindow;
    };
}


