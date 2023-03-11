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
        : m_implementation(_appInfo)
    {
    }

    GraphicsContext::~GraphicsContext() = default;

    Window *GraphicsContext::GetWindow() const
    {
        return m_implementation.GetWindow();
    }

    bool GraphicsContext::EndFrame()
    {
        return GetWindow()->WaitForEvents();
    }
}
