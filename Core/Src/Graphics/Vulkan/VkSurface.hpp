/**
 * @file
 * @author Max Godefroy
 * @date 21/03/2022.
 */

#pragma once

#include "Graphics/Vulkan/CommonStructures.hpp"
#include "Graphics/Vulkan/VkHeaders.hpp"

struct GLFWwindow;

namespace KryneEngine
{
    class VkSurface
    {
    public:
        struct Capabilities
        {
            VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
            DynamicArray<VkSurfaceFormatKHR> m_formats;
            DynamicArray<VkPresentModeKHR> m_presentModes;
        };

        VkSurface(VkInstance _instance, GLFWwindow *_window);

        virtual ~VkSurface();

        void Destroy(VkInstance _instance);

        void UpdateCapabilities(const VkPhysicalDevice& _physicalDevice);

        [[nodiscard]] const VkSurfaceKHR& GetSurface() const { return m_surface; }
        [[nodiscard]] const Capabilities& GetCapabilities() const { return m_capabilities; }

    private:
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        Capabilities m_capabilities;
    };
}
