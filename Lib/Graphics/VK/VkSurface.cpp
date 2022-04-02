/**
 * @file
 * @author Max Godefroy
 * @date 21/03/2022.
 */

#include "VkSurface.hpp"

#include <Graphics/VK/HelperFunctions.hpp>
#include <GLFW/glfw3.h>

namespace KryneEngine
{
    using VkHelperFunctions::VkAssert;

    VkSurface::VkSurface(VkSharedInstanceRef &&_instanceRef, GLFWwindow *_window)
        : m_sharedInstanceRef(eastl::move(_instanceRef))
    {
        VkAssert(glfwCreateWindowSurface(*m_sharedInstanceRef,
                                         _window,
                                         nullptr,
                                         reinterpret_cast<VkSurfaceKHR*>(&m_surface)));
    }

    void VkSurface::UpdateCapabilities(const vk::PhysicalDevice& _physicalDevice)
    {
        VkAssert(_physicalDevice.getSurfaceCapabilitiesKHR(m_surface,&m_capabilities.m_surfaceCapabilities));

        auto formats = _physicalDevice.getSurfaceFormatsKHR(m_surface);
        eastl::copy(formats.begin(), formats.end(), eastl::back_inserter(m_capabilities.m_formats));

        auto presentModes = _physicalDevice.getSurfacePresentModesKHR(m_surface);
        eastl::copy(presentModes.begin(), presentModes.end(),
                eastl::back_inserter(m_capabilities.m_presentModes));

        Assert(!formats.empty() && !presentModes.empty());
    }

    VkSurface::~VkSurface()
    {
        m_sharedInstanceRef->destroy(m_surface);
    }
}