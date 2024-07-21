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
    VkSurface::VkSurface(VkInstance _instance, GLFWwindow *_window)
    {
        VkAssert(glfwCreateWindowSurface(_instance,
                                         _window,
                                         nullptr,
                                         reinterpret_cast<VkSurfaceKHR*>(&m_surface)));
    }

    VkSurface::~VkSurface()
    {
        KE_ASSERT(!m_surface);
    }

    void VkSurface::Destroy(VkInstance _instance)
    {
        vkDestroySurfaceKHR(_instance, SafeReset(m_surface), nullptr);
    }

    void VkSurface::UpdateCapabilities(const VkPhysicalDevice& _physicalDevice)
    {
        VkAssert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, m_surface, &m_capabilities.m_surfaceCapabilities));

        VkHelperFunctions::VkArrayFetch(m_capabilities.m_formats, vkGetPhysicalDeviceSurfaceFormatsKHR, _physicalDevice, m_surface);
        VkHelperFunctions::VkArrayFetch(m_capabilities.m_presentModes, vkGetPhysicalDeviceSurfacePresentModesKHR, _physicalDevice, m_surface);

        KE_ASSERT(!m_capabilities.m_formats.Empty() && !m_capabilities.m_presentModes.Empty());
    }
}