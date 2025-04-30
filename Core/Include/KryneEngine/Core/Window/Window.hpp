/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <EASTL/vector_map.h>
#include <EASTL/unique_ptr.h>
#include <GLFW/glfw3.h>

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Graphics/GraphicsCommon.hpp"
#include "KryneEngine/Core/Threads/LightweightMutex.hpp"

namespace KryneEngine
{
    class GraphicsContext;
    class InputManager;

    class Window
    {
    public:

        explicit Window(const GraphicsCommon::ApplicationInfo& _appInfo, AllocatorInstance _allocator);

        virtual ~Window();

        [[nodiscard]] bool WaitForEvents() const;
        [[nodiscard]] GLFWwindow* GetGlfwWindow() const { return m_glfwWindow; }
        [[nodiscard]] GraphicsContext* GetGraphicsContext() const { return m_graphicsContext; }
        [[nodiscard]] InputManager* GetInputManager() const { return m_inputManager; }

        [[nodiscard]] u32 RegisterWindowFocusEventCallback(eastl::function<void(bool)>&& _callback);
        void UnregisterWindowFocusEventCallback(u32 _id);

    private:
        AllocatorInstance m_allocator;
        GLFWwindow* m_glfwWindow;

        GraphicsContext* m_graphicsContext;
        InputManager* m_inputManager;

        LightweightMutex m_callbackMutex;

        static void WindowFocusCallback(GLFWwindow* _window, s32 _focused);
        eastl::vector_map<u32, eastl::function<void(bool)>> m_windowFocusEventListeners;
        u32 m_windowFocusEventCounter = 0;
    };
}


