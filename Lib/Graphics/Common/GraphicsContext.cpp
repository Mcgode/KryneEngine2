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

    GraphicsContext::~GraphicsContext()
    {
        WaitForLastFrame();
    }

    bool GraphicsContext::EndFrame()
    {
        m_implementation.EndFrame(m_frameId);
        m_frameId++;
        return GetWindow()->WaitForEvents();
    }

    void GraphicsContext::WaitForLastFrame() const
    {
        m_implementation.WaitForFrame(m_frameId - 1);
    }
}
