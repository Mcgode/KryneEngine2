/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <EASTL/unique_ptr.h>
#include <Graphics/Common/GraphicsCommon.hpp>

namespace KryneEngine
{
    class Window;
    class VkGraphicsContext;

    class GraphicsContext
    {
    public:
        explicit GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo);

        ~GraphicsContext();

        [[nodiscard]] Window* GetWindow() const;

        bool EndFrame();

    private:
#if defined(KE_GRAPHICS_API_VK)
        eastl::unique_ptr<VkGraphicsContext> m_implementation;
#endif
    };
}


