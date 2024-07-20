/**
 * @file
 * @author Max Godefroy
 * @date 20/03/2022.
 */

#pragma once

#include "CommonStructures.hpp"
#include "VkHeaders.hpp"
#include <EASTL/shared_ptr.h>
#include <EASTL/span.h>

struct GLFWwindow;

namespace KryneEngine
{
    class VkSurface;
    struct VkResources;
    class VkDebugHandler;

    class VkSwapChain
    {
        friend class VkGraphicsContext;

    public:
        VkSwapChain(const GraphicsCommon::ApplicationInfo &_appInfo,
                    vk::Device _device,
                    const VkSurface &_surface,
                    VkResources &_resources,
                    GLFWwindow *_window,
                    const VkCommonStructures::QueueIndices &_queueIndices,
                    u64 _currentFrameIndex,
                    VkSwapChain *_oldSwapChain);

        virtual ~VkSwapChain();

        void AcquireNextImage(vk::Device _device, u8 _frameIndex);

        void Present(vk::Queue _presentQueue, const eastl::span<vk::Semaphore> &_semaphores);

        void Destroy(vk::Device _device, VkResources& _resources);

#if !defined(KE_FINAL)
        void SetDebugHandler(const eastl::shared_ptr<VkDebugHandler> &_handler, VkDevice _device);
#endif

    private:
        vk::SwapchainKHR m_swapChain;
        vk::SharingMode m_sharingMode;
        DynamicArray<GenPool::Handle> m_renderTargetTextures;
        DynamicArray<GenPool::Handle> m_renderTargetViews;
        DynamicArray<vk::Semaphore> m_imageAvailableSemaphores;
        u32 m_imageIndex = 0;
    };
}
