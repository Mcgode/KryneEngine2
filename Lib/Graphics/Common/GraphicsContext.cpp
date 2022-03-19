/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "GraphicsContext.hpp"

#include <Graphics/Common/Window.hpp>

namespace KryneEngine
{
    GraphicsContext::GraphicsContext()
        : m_window(eastl::make_unique<Window>(Window::Params()))
    {
    }

    bool GraphicsContext::EndFrame()
    {
        return m_window->WaitForEvents();
    }
}
