/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <GLFW/glfw3.h>
#include <Common/KETypes.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>

namespace KryneEngine
{
    class Window
    {
    public:

        explicit Window(const GraphicsCommon::ApplicationInfo& _appInfo);

        virtual ~Window();

        [[nodiscard]] bool WaitForEvents() const;
        [[nodiscard]] GLFWwindow* GetGlfwWindow() const { return m_glfwWindow; }

    private:
        GLFWwindow* m_glfwWindow;
    };
}


