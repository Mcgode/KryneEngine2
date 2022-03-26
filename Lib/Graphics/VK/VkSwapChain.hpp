/**
 * @file
 * @author Max Godefroy
 * @date 20/03/2022.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <Common/KETypes.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Graphics/VK/CommonStructures.hpp>

struct GLFWwindow;

namespace KryneEngine
{
    class VkSurface;

    class VkSwapChain
    {
    public:
        VkSwapChain(const GraphicsCommon::ApplicationInfo &_appInfo,
                    VkSharedDeviceRef &&_deviceRef,
                    const VkSurface *_surface, GLFWwindow *_window,
                    const VkCommonStructures::QueueIndices &_queueIndices, VkSwapChain *_oldSwapChain);

        virtual ~VkSwapChain();

    private:
        vk::SwapchainKHR m_swapChain;
        VkSharedDeviceRef m_deviceRef;
        vk::SharingMode m_sharingMode;
    };
}
