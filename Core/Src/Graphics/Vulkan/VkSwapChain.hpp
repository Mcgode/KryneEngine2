/**
 * @file
 * @author Max Godefroy
 * @date 20/03/2022.
 */

#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/span.h>

#include "Graphics/Vulkan/CommonStructures.hpp"
#include "Graphics/Vulkan/VkHeaders.hpp"
#include "KryneEngine/Core/Graphics/Common/Handles.hpp"

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
        explicit VkSwapChain(AllocatorInstance _allocator);

        void Init(const GraphicsCommon::ApplicationInfo &_appInfo,
                    VkDevice _device,
                    const VkSurface &_surface,
                    VkResources &_resources,
                    GLFWwindow *_window,
                    const VkCommonStructures::QueueIndices &_queueIndices,
                    u64 _currentFrameIndex,
                    VkSwapChain *_oldSwapChain);

        virtual ~VkSwapChain();

        void AcquireNextImage(VkDevice _device, u8 _frameIndex);

        void Present(VkQueue _presentQueue, const eastl::span<VkSemaphore> &_semaphores);

        void Destroy(VkDevice _device, VkResources& _resources);

#if !defined(KE_FINAL)
        void SetDebugHandler(const eastl::shared_ptr<VkDebugHandler> &_handler, VkDevice _device);
#endif

    private:
        VkSwapchainKHR m_swapChain;
        VkSharingMode m_sharingMode;
        DynamicArray<TextureHandle> m_renderTargetTextures;
        DynamicArray<RenderTargetViewHandle> m_renderTargetViews;
        DynamicArray<VkSemaphore> m_imageAvailableSemaphores;
        u32 m_imageIndex = 0;
    };
}
