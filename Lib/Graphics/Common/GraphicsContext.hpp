/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#if defined(KE_GRAPHICS_API_VK)
#include <Graphics/VK/VkGraphicsContext.hpp>
#endif

#include <EASTL/unique_ptr.h>



namespace KryneEngine
{
    class Window;

    class GraphicsContext
    {
    public:
        GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo);

        bool EndFrame();

    private:
        eastl::unique_ptr<Window> m_window;

#if defined(KE_GRAPHICS_API_VK)
        VkGraphicsContext m_implementation;
#endif
    };
}


