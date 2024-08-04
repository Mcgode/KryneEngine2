/**
 * @file
 * @author Max Godefroy
 * @date 20/03/2022.
 */

#include "VkSwapChain.hpp"
#include "Common/EastlHelpers.hpp"
#include "VkResources.hpp"
#include "VkDebugHandler.hpp"

#include <Common/Assert.hpp>
#include <Graphics/VK/HelperFunctions.hpp>
#include <Graphics/VK/VkSurface.hpp>
#include <GLFW/glfw3.h>
#include <Graphics/Common/ResourceViews/RenderTargetView.hpp>

namespace KryneEngine
{
    VkSwapChain::VkSwapChain(
            const GraphicsCommon::ApplicationInfo &_appInfo,
            VkDevice _device, const VkSurface &_surface,
            VkResources &_resources, GLFWwindow *_window,
            const VkCommonStructures::QueueIndices &_queueIndices,
            u64 _currentFrameIndex,
            VkSwapChain *_oldSwapChain)
    {
        const auto& capabilities = _surface.GetCapabilities();
        KE_ASSERT(!capabilities.m_formats.Empty() && !capabilities.m_presentModes.Empty());

        const auto displayOptions = _appInfo.m_displayOptions;

        // Select appropriate format
        VkSurfaceFormatKHR selectedSurfaceFormat;
        if (displayOptions.m_sRgbPresent != GraphicsCommon::SoftEnable::Disabled)
        {
            for (const auto& format: capabilities.m_formats)
            {
                if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    selectedSurfaceFormat = format;
                    break;
                }
            }

            KE_ASSERT(displayOptions.m_sRgbPresent == GraphicsCommon::SoftEnable::TryEnable
                      || selectedSurfaceFormat.format != VK_FORMAT_UNDEFINED);
        }
        if (selectedSurfaceFormat.format == VK_FORMAT_UNDEFINED)
        {
            selectedSurfaceFormat = capabilities.m_formats[0];
        }

        // Select appropriate present mode
        VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        if (displayOptions.m_tripleBuffering != GraphicsCommon::SoftEnable::Disabled)
        {
            for (const auto& presentMode: capabilities.m_presentModes)
            {
                if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    selectedPresentMode = presentMode;
                    break;
                }
            }

            KE_ASSERT(displayOptions.m_tripleBuffering == GraphicsCommon::SoftEnable::TryEnable
                   || selectedPresentMode != VK_PRESENT_MODE_FIFO_KHR);
        }

        // Retrieve extent
        VkExtent2D extent;
        if (capabilities.m_surfaceCapabilities.currentExtent.width != std::numeric_limits<u32>::max()
            && capabilities.m_surfaceCapabilities.currentExtent.height != std::numeric_limits<u32>::max())
        {
            extent = capabilities.m_surfaceCapabilities.currentExtent;
        }
        else
        {
            s32 width, height;
            glfwGetFramebufferSize(_window, &width, &height);

            extent = VkExtent2D {
                static_cast<u32>(width),
                static_cast<u32>(height)
            };

            extent.width = eastl::clamp(extent.width,
                                        capabilities.m_surfaceCapabilities.minImageExtent.width,
                                        capabilities.m_surfaceCapabilities.maxImageExtent.width);
            extent.height = eastl::clamp(extent.height,
                                         capabilities.m_surfaceCapabilities.minImageExtent.height,
                                         capabilities.m_surfaceCapabilities.maxImageExtent.height);
        }

        u32 desiredImageCount = 2;
        if (displayOptions.m_tripleBuffering != GraphicsCommon::SoftEnable::Disabled)
        {
            desiredImageCount++;
        }
        desiredImageCount = eastl::max(desiredImageCount, capabilities.m_surfaceCapabilities.minImageCount);
        if (capabilities.m_surfaceCapabilities.minImageCount != 0)
        {
            desiredImageCount = eastl::min(desiredImageCount, capabilities.m_surfaceCapabilities.maxImageCount);
        }
        KE_ASSERT(desiredImageCount >= 3 || displayOptions.m_tripleBuffering != GraphicsCommon::SoftEnable::ForceEnabled);

        eastl::vector<u32> queueFamilyIndices{};
        m_sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (_appInfo.m_features.m_concurrentQueues)
        {
            queueFamilyIndices = _queueIndices.RetrieveDifferentFamilies();
            if (queueFamilyIndices.size() <= 1)
            {
                queueFamilyIndices.clear();
            }
            else
            {
                m_sharingMode = VK_SHARING_MODE_CONCURRENT;
            }
        }

        {
            VkSwapchainCreateInfoKHR createInfo{
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .flags = 0,
                    .surface = _surface.GetSurface(),
                    .minImageCount = desiredImageCount,
                    .imageFormat = selectedSurfaceFormat.format,
                    .imageColorSpace = selectedSurfaceFormat.colorSpace,
                    .imageExtent = extent,
                    .imageArrayLayers = 1,
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    .imageSharingMode = m_sharingMode,
                    .queueFamilyIndexCount = static_cast<u32>(queueFamilyIndices.size()),
                    .pQueueFamilyIndices = queueFamilyIndices.data(),
                    .preTransform = capabilities.m_surfaceCapabilities.currentTransform,
                    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                    .presentMode = selectedPresentMode,
                    .oldSwapchain = _oldSwapChain == nullptr ? VK_NULL_HANDLE : _oldSwapChain->m_swapChain};

            VkAssert(vkCreateSwapchainKHR(_device, &createInfo, nullptr, &m_swapChain));
        }

        {
            u32 imageCount;
            VkAssert(vkGetSwapchainImagesKHR(_device, m_swapChain,  &imageCount, nullptr));
            DynamicArray<VkImage> images;
            images.Resize(imageCount);
            VkAssert(vkGetSwapchainImagesKHR(_device, m_swapChain, &imageCount, images.Data()));
            KE_ASSERT_MSG(imageCount > 0, "Unable to retrieve swapchain images");

            m_renderTargetTextures.Resize(imageCount);
            m_renderTargetViews.Resize(imageCount);
            m_imageAvailableSemaphores.Resize(imageCount);
            for (auto i = 0u; i < imageCount; i++)
            {
                const auto textureHandle = _resources.RegisterTexture(images[i], {extent.width, extent.height, 1});
#if !defined(KE_FINAL)
                const eastl::string rtvDebugName = _appInfo.m_applicationName + "/Swapchain/RTV[" + eastl::to_string(i) + "]";
#endif
                const RenderTargetViewDesc rtvDesc {
                    .m_texture = textureHandle,
                    .m_format = VkHelperFunctions::FromVkFormat(selectedSurfaceFormat.format),
#if !defined(KE_FINAL)
                    .m_debugName = rtvDebugName,
#endif
                };

                m_renderTargetTextures.Init(i, textureHandle);
                m_renderTargetViews.Init(i, _resources.CreateRenderTargetView(rtvDesc, _device));
                {
                    VkSemaphoreCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
                    VkAssert(vkCreateSemaphore(_device, &createInfo, nullptr, &m_imageAvailableSemaphores[i]));
                }
            }
        }

        AcquireNextImage(_device, _currentFrameIndex % m_imageAvailableSemaphores.Size());
    }

    VkSwapChain::~VkSwapChain()
    {
        KE_ASSERT(!m_swapChain);
    }

    void VkSwapChain::AcquireNextImage(VkDevice _device, u8 _frameIndex)
    {
        VkAssert(vkAcquireNextImageKHR(
                _device,
                m_swapChain,
                UINT64_MAX,
                m_imageAvailableSemaphores[_frameIndex],
                VK_NULL_HANDLE,
                &m_imageIndex));
    }

    void VkSwapChain::Present(VkQueue _presentQueue, const eastl::span<VkSemaphore> &_semaphores)
    {
	    const VkPresentInfoKHR presentInfo = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = static_cast<uint32_t>(_semaphores.size()),
                .pWaitSemaphores = _semaphores.data(),
                .swapchainCount = 1,
                .pSwapchains = &m_swapChain,
                .pImageIndices = &m_imageIndex,
        };

        VkAssert(vkQueuePresentKHR(_presentQueue, &presentInfo));
    }

    void VkSwapChain::Destroy(VkDevice _device, VkResources &_resources)
    {
        for (const auto handle: m_renderTargetViews)
        {
            KE_ASSERT_MSG(_resources.FreeRenderTargetView(handle, _device),
                          "Handle was invalid. It shouldn't. Something went wrong with the lifecycle.");
        }
        m_renderTargetViews.Clear();

        for (const auto handle: m_renderTargetTextures)
        {
            // Free the texture from the gen pool, but don't do a release of the VkImage, as it's handled
            // by the swapchain
            KE_ASSERT_MSG(_resources.ReleaseTexture(handle, _device, false),
                          "Handle was invalid. It shouldn't. Something went wrong with the lifecycle.");
        }
        m_renderTargetTextures.Clear();

        for (auto semaphore: m_imageAvailableSemaphores)
        {
            vkDestroySemaphore(_device, semaphore, nullptr);
        }

        vkDestroySwapchainKHR(_device, SafeReset(m_swapChain), nullptr);
    }

#if !defined(KE_FINAL)
    void VkSwapChain::SetDebugHandler(const eastl::shared_ptr<VkDebugHandler> &_handler, VkDevice _device)
    {
        {
            eastl::string name = "Swapchain";
            _handler->SetName(_device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (u64)(VkSwapchainKHR)m_swapChain, name);
        }

        DynamicArray<VkImage> imageArray;
        u32 imageCount = m_imageAvailableSemaphores.Size();
        imageArray.Resize(imageCount);
        vkGetSwapchainImagesKHR(_device, m_swapChain, &imageCount, imageArray.Data());
        for (auto i = 0u; i < imageCount; i++)
        {
            {
                const eastl::string name = "Swapchain/Texture[" + eastl::to_string(i) + "]";
                _handler->SetName(_device, VK_OBJECT_TYPE_IMAGE, (u64) imageArray[i], name);
            }

            {
                const eastl::string name = "Swapchain/ImageAvailableSemaphore[" + eastl::to_string(i) + "]";
                _handler->SetName(_device, VK_OBJECT_TYPE_SEMAPHORE, (u64)(VkSemaphore) m_imageAvailableSemaphores[i], name);
            }
        }
    }
#endif
}
