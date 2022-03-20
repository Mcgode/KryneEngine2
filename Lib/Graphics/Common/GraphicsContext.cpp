/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "GraphicsContext.hpp"

#include <Graphics/Common/Window.hpp>

namespace KryneEngine
{
    GraphicsContext::GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo)
        : m_window(eastl::make_unique<Window>(Window::Params()))
        , m_implementation(_appInfo)
    {
    }

    bool GraphicsContext::EndFrame()
    {
        return m_window->WaitForEvents();
    }
}
