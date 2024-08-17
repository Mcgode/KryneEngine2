/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <Common/Types.hpp>
#include <EASTL/unique_ptr.h>
#include <EASTL/vector_map.h>
#include <GLFW/glfw3.h>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Threads/LightweightMutex.hpp>

namespace KryneEngine
{
    class GraphicsContext;
    class InputManager;

    class Window
    {
    public:

        explicit Window(const GraphicsCommon::ApplicationInfo& _appInfo);

        virtual ~Window();

        [[nodiscard]] bool WaitForEvents() const;
        [[nodiscard]] GLFWwindow* GetGlfwWindow() const { return m_glfwWindow; }
        [[nodiscard]] GraphicsContext* GetGraphicsContext() const { return m_graphicsContext.get(); }
        [[nodiscard]] InputManager* GetInputManager() const { return m_inputManager.get(); }

        [[nodiscard]] u32 RegisterWindowFocusEventCallback(eastl::function<void(bool)>&& _callback);
        void UnregisterWindowFocusEventCallback(u32 _id);

    private:
        GLFWwindow* m_glfwWindow;

        eastl::unique_ptr<GraphicsContext> m_graphicsContext;
        eastl::unique_ptr<InputManager> m_inputManager;

        LightweightMutex m_callbackMutex;

        static void WindowFocusCallback(GLFWwindow* _window, s32 _focused);
        eastl::vector_map<u32, eastl::function<void(bool)>> m_windowFocusEventListeners;
        u32 m_windowFocusEventCounter = 0;
    };
}


