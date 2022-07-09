/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "GraphicsContext.hpp"

#if defined(KE_GRAPHICS_API_VK)
#include <Graphics/VK/VkGraphicsContext.hpp>
#endif

#include <Graphics/Common/Window.hpp>


namespace KryneEngine
{
    GraphicsContext::GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo)
        : m_implementation(eastl::make_unique<VkGraphicsContext>(_appInfo))
    {
    }

    GraphicsContext::~GraphicsContext() = default;

    Window *GraphicsContext::GetWindow() const
    {
        return m_implementation->GetWindow();
    }

    bool GraphicsContext::EndFrame()
    {
        return GetWindow()->WaitForEvents();
    }
}
