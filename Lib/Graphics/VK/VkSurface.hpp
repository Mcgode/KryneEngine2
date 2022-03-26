/**
 * @file
 * @author Max Godefroy
 * @date 21/03/2022.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <Common/KETypes.hpp>
#include <Graphics/VK/CommonStructures.hpp>

struct GLFWwindow;

namespace KryneEngine
{
    class VkSurface
    {
    public:
        struct Capabilities
        {
            vk::SurfaceCapabilitiesKHR m_surfaceCapabilities;
            eastl::vector<vk::SurfaceFormatKHR> m_formats;
            eastl::vector<vk::PresentModeKHR> m_presentModes;
        };

        VkSurface(VkSharedInstanceRef &&_instanceRef, GLFWwindow *_window);

        virtual ~VkSurface();

        void UpdateCapabilities(const vk::PhysicalDevice& _physicalDevice);

        [[nodiscard]] const vk::SurfaceKHR& GetSurface() const { return m_surface; }
        [[nodiscard]] const Capabilities& GetCapabilities() const { return m_capabilities; }

    private:
        vk::SurfaceKHR m_surface;
        VkSharedInstanceRef m_sharedInstanceRef;
        Capabilities m_capabilities;
    };
}
