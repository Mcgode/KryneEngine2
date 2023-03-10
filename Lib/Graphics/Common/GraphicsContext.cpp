/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "GraphicsContext.hpp"

#if defined(KE_GRAPHICS_API_VK)
#include <Graphics/VK/VkGraphicsContext.hpp>
#elif defined(KE_GRAPHICS_API_DX12)
#error Not yet implemented
#else
#error No valid graphics API
#endif

#include <Graphics/Common/Window.hpp>


namespace KryneEngine
{
    GraphicsContext::GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo)
#if defined(KE_GRAPHICS_API_VK)
        : m_implementation(eastl::make_unique<VkGraphicsContext>(_appInfo))
#elif defined(KE_GRAPHICS_API_DX12)
#error Not yet implemented
#endif
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
