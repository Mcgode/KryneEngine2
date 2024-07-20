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
#include <Graphics/Common/RenderTargetView.hpp>

namespace KryneEngine
{
    VkSwapChain::VkSwapChain(
            const GraphicsCommon::ApplicationInfo &_appInfo,
            vk::Device _device, const VkSurface &_surface,
            VkResources &_resources, GLFWwindow *_window,
            const VkCommonStructures::QueueIndices &_queueIndices,
            u64 _currentFrameIndex,
            const eastl::shared_ptr<VkDebugHandler> &_debugHandler,
            VkSwapChain *_oldSwapChain)
    {
#if !defined(KE_FINAL)
        m_debugHandler = _debugHandler;
#endif

        const auto& capabilities = _surface.GetCapabilities();
        KE_ASSERT(!capabilities.m_formats.empty() && !capabilities.m_presentModes.empty());

        const auto displayOptions = _appInfo.m_displayOptions;

        // Select appropriate format
        vk::SurfaceFormatKHR selectedSurfaceFormat;
        if (displayOptions.m_sRgbPresent != GraphicsCommon::SoftEnable::Disabled)
        {
            for (const auto& format: capabilities.m_formats)
            {
                if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                {
                    selectedSurfaceFormat = format;
                    break;
                }
            }

            KE_ASSERT(displayOptions.m_sRgbPresent == GraphicsCommon::SoftEnable::TryEnable
                      || selectedSurfaceFormat.format != vk::Format::eUndefined);
        }
        if (selectedSurfaceFormat.format == vk::Format::eUndefined)
        {
            selectedSurfaceFormat = capabilities.m_formats[0];
        }

        // Select appropriate present mode
        vk::PresentModeKHR selectedPresentMode = vk::PresentModeKHR::eFifo;
        if (displayOptions.m_tripleBuffering != GraphicsCommon::SoftEnable::Disabled)
        {
            for (const auto& presentMode: capabilities.m_presentModes)
            {
                if (presentMode == vk::PresentModeKHR::eMailbox)
                {
                    selectedPresentMode = presentMode;
                    break;
                }
            }

            KE_ASSERT(displayOptions.m_tripleBuffering == GraphicsCommon::SoftEnable::TryEnable
                   || selectedPresentMode != vk::PresentModeKHR::eFifo);
        }

        // Retrieve extent
        vk::Extent2D extent;
        if (capabilities.m_surfaceCapabilities.currentExtent != std::numeric_limits<u32>::max())
        {
            extent = capabilities.m_surfaceCapabilities.currentExtent;
        }
        else
        {
            s32 width, height;
            glfwGetFramebufferSize(_window, &width, &height);

            extent = vk::Extent2D {
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

        u32 imageCount = 2;
        if (displayOptions.m_tripleBuffering != GraphicsCommon::SoftEnable::Disabled)
        {
            imageCount++;
        }
        imageCount = eastl::max(imageCount, capabilities.m_surfaceCapabilities.minImageCount);
        if (capabilities.m_surfaceCapabilities.minImageCount != 0)
        {
            imageCount = eastl::min(imageCount, capabilities.m_surfaceCapabilities.maxImageCount);
        }
        KE_ASSERT(imageCount >= 3 || displayOptions.m_tripleBuffering != GraphicsCommon::SoftEnable::ForceEnabled);

        eastl::vector<u32> queueFamilyIndices{};
        m_sharingMode = vk::SharingMode::eExclusive;
        if (_appInfo.m_features.m_concurrentQueues)
        {
            queueFamilyIndices = _queueIndices.RetrieveDifferentFamilies();
            if (queueFamilyIndices.size() <= 1)
            {
                queueFamilyIndices.clear();
            }
            else
            {
                m_sharingMode = vk::SharingMode::eConcurrent;
            }
        }

        vk::SwapchainCreateInfoKHR createInfo(
                {},
                _surface.GetSurface(),
                imageCount,
                selectedSurfaceFormat.format,
                selectedSurfaceFormat.colorSpace,
                extent,
                1,
                vk::ImageUsageFlagBits::eColorAttachment,
                m_sharingMode,
                VkHelperFunctions::MakeArrayProxy(queueFamilyIndices),
                capabilities.m_surfaceCapabilities.currentTransform,
                vk::CompositeAlphaFlagBitsKHR::eOpaque,
                selectedPresentMode,
                true,
                _oldSwapChain == nullptr ? vk::SwapchainKHR{} : _oldSwapChain->m_swapChain);

        VkAssert(_device.createSwapchainKHR(&createInfo, nullptr, &m_swapChain));
#if !defined(KE_FINAL)
        {
            eastl::string name = _appInfo.m_applicationName + "/Swapchain";
            m_debugHandler->SetName(_device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (u64)(VkSwapchainKHR)m_swapChain, name);
        }
#endif

        {
            const auto images = _device.getSwapchainImagesKHR(m_swapChain);
            KE_ASSERT_MSG(!images.empty(), "Unable to retrieve swapchain images");

            m_renderTargetTextures.Resize(images.size());
            m_renderTargetViews.Resize(images.size());
            m_imageAvailableSemaphores.Resize(images.size());
            for (auto i = 0u; i < images.size(); i++)
            {
                const auto textureHandle = _resources.RegisterTexture(
                        images[i],
                        { u16(extent.width), u16(extent.height) });
#if !defined(KE_FINAL)
                const eastl::string imageDebugName = _appInfo.m_applicationName + "/Swapchain/Texture[" + eastl::to_string(i) + "]";
                {
                    const u64 imageHandle = (u64)(VkImage)images[i];
                    m_debugHandler->SetName(_device, VK_OBJECT_TYPE_IMAGE, imageHandle, imageDebugName);
                }

                const eastl::string rtvDebugName = _appInfo.m_applicationName + "/Swapchain/RTV[" + eastl::to_string(i) + "]";
#endif
                const RenderTargetViewDesc rtvDesc {
                    .m_textureHandle = textureHandle,
                    .m_format = VkHelperFunctions::FromVkFormat(selectedSurfaceFormat.format),
#if !defined(KE_FINAL)
                    .m_debugName = rtvDebugName,
#endif
                };

                m_renderTargetTextures.Init(i, textureHandle);
                m_renderTargetViews.Init(i, _resources.CreateRenderTargetView(rtvDesc, _device));
                m_imageAvailableSemaphores[i] = _device.createSemaphore(vk::SemaphoreCreateInfo{});
            }
        }

        AcquireNextImage(_device, _currentFrameIndex % m_imageAvailableSemaphores.Size());
    }

    VkSwapChain::~VkSwapChain()
    {
        KE_ASSERT(!m_swapChain);
    }

    void VkSwapChain::AcquireNextImage(vk::Device _device, u8 _frameIndex)
    {
        m_imageIndex = _device.acquireNextImageKHR(m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[_frameIndex], nullptr).value;
    }

    void VkSwapChain::Present(vk::Queue _presentQueue, const eastl::span<vk::Semaphore> &_semaphores)
    {
	    const vk::PresentInfoKHR presentInfo = {
        	_semaphores,
        	m_swapChain,
            m_imageIndex
        };

        VkAssert(_presentQueue.presentKHR(presentInfo));
    }

    void VkSwapChain::Destroy(vk::Device _device, VkResources &_resources)
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

        SafeDestroy(_device, m_swapChain);
    }
}
