/**
 * @file
 * @author Max Godefroy
 * @date 21/03/2022.
 */

#include "VkSurface.hpp"

#include <GLFW/glfw3.h>
#include <Graphics/VK/HelperFunctions.hpp>

namespace KryneEngine
{
    VkSurface::VkSurface(VkInstance _instance, GLFWwindow *_window)
    {
        KE_ZoneScopedFunction("VkSurface::VkSurface");
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
        KE_ZoneScopedFunction("VkSurface::UpdateCapabilities");

        VkAssert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, m_surface, &m_capabilities.m_surfaceCapabilities));

        VkHelperFunctions::VkArrayFetch(m_capabilities.m_formats, vkGetPhysicalDeviceSurfaceFormatsKHR, _physicalDevice, m_surface);
        VkHelperFunctions::VkArrayFetch(m_capabilities.m_presentModes, vkGetPhysicalDeviceSurfacePresentModesKHR, _physicalDevice, m_surface);

        KE_ASSERT(!m_capabilities.m_formats.Empty() && !m_capabilities.m_presentModes.Empty());
    }
}