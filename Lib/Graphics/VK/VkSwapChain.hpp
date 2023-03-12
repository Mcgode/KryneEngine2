/**
 * @file
 * @author Max Godefroy
 * @date 20/03/2022.
 */

#pragma once

#include "VkHeaders.hpp"
#include "VkTexture.hpp"

struct GLFWwindow;

namespace KryneEngine
{
    class VkSurface;

    class VkSwapChain
    {
        friend class VkGraphicsContext;

    public:
        VkSwapChain(const GraphicsCommon::ApplicationInfo &_appInfo,
                    VkSharedDeviceRef &&_deviceRef,
                    const VkSurface *_surface,
                    GLFWwindow *_window,
                    const VkCommonStructures::QueueIndices &_queueIndices,
                    VkSwapChain *_oldSwapChain = nullptr);

        virtual ~VkSwapChain();

    private:
        vk::SwapchainKHR m_swapChain;
        VkSharedDeviceRef m_deviceRef;
        vk::SharingMode m_sharingMode;
        DynamicArray<VkTexture> m_swapChainTextures;
    };
}
