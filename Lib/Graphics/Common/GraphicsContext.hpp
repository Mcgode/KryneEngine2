/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <EASTL/unique_ptr.h>

#if defined(KE_GRAPHICS_API_VK)
    #include <Graphics/VK/VkGraphicsContext.hpp>
#endif

namespace KryneEngine
{
    class Window;

    class GraphicsContext
    {
    public:
        GraphicsContext();

        bool EndFrame();

    private:
        eastl::unique_ptr<Window> m_window;

#if defined(KE_GRAPHICS_API_VK)
        VkGraphicsContext m_implementation;
#endif
    };
}


