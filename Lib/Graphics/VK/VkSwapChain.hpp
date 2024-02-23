/**
 * @file
 * @author Max Godefroy
 * @date 20/03/2022.
 */

#pragma once

#include <EASTL/span.h>
#include "VkHeaders.hpp"
#include "CommonStructures.hpp"

struct GLFWwindow;

namespace KryneEngine
{
    class VkSurface;
    struct VkResources;

    class VkSwapChain
    {
        friend class VkGraphicsContext;

    public:
        VkSwapChain(const GraphicsCommon::ApplicationInfo &_appInfo,
                    VkSharedDeviceRef &&_deviceRef,
                    const VkSurface &_surface,
                    VkResources &_resources,
                    GLFWwindow *_window,
                    const VkCommonStructures::QueueIndices &_queueIndices,
                    VkSwapChain *_oldSwapChain = nullptr);

        virtual ~VkSwapChain();

        vk::Semaphore AcquireNextImage(u8 _frameIndex);

        void Present(vk::Queue _presentQueue, const eastl::span<vk::Semaphore>& _semaphores);

        void Destroy(vk::Device _device, VkResources& _resources);

    private:
        vk::SwapchainKHR m_swapChain;
        VkSharedDeviceRef m_deviceRef;
        vk::SharingMode m_sharingMode;
        DynamicArray<GenPool::Handle> m_renderTargetTextures;
        DynamicArray<GenPool::Handle> m_renderTargetViews;
        DynamicArray<vk::Semaphore> m_imageAvailableSemaphores;
        u32 m_imageIndex = 0;
    };
}
