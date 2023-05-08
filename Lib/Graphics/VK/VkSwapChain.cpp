/**
 * @file
 * @author Max Godefroy
 * @date 20/03/2022.
 */

#include "VkSwapChain.hpp"
#include "Common/KEEastlHelpers.hpp"

#include <Common/Assert.hpp>
#include <Graphics/VK/HelperFunctions.hpp>
#include <Graphics/VK/VkSurface.hpp>
#include <GLFW/glfw3.h>

namespace KryneEngine
{
    VkSwapChain::VkSwapChain(const GraphicsCommon::ApplicationInfo &_appInfo, VkSharedDeviceRef &&_deviceRef,
                             const VkSurface *_surface, GLFWwindow *_window,
                             const VkCommonStructures::QueueIndices &_queueIndices, VkSwapChain *_oldSwapChain)
        : m_deviceRef(eastl::move(_deviceRef))
    {
        const auto& capabilities = _surface->GetCapabilities();
        KE_ASSERT(!capabilities.m_formats.empty() && !capabilities.m_presentModes.empty());

        const auto displayOptions = _appInfo.m_displayOptions;

        // Select appropriate format
        vk::SurfaceFormatKHR selectedFormat;
        if (displayOptions.m_sRgbPresent != GraphicsCommon::SoftEnable::Disabled)
        {
            for (const auto& format: capabilities.m_formats)
            {
                if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                {
                    selectedFormat = format;
                    break;
                }
            }

            KE_ASSERT(displayOptions.m_sRgbPresent == GraphicsCommon::SoftEnable::TryEnable
                || selectedFormat.format != vk::Format::eUndefined);
        }
        if (selectedFormat.format == vk::Format::eUndefined)
        {
            selectedFormat = capabilities.m_formats[0];
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
                _surface->GetSurface(),
                imageCount,
                selectedFormat.format,
                selectedFormat.colorSpace,
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

        VkAssert(m_deviceRef->createSwapchainKHR(&createInfo, nullptr, &m_swapChain));

        {
            const auto images = m_deviceRef->getSwapchainImagesKHR(m_swapChain);
            KE_ASSERT_MSG(!images.empty(), "Unable to retrieve swapchain images");

            Texture::Options textureOptions = {
                    VkHelperFunctions::FromVkFormat(selectedFormat.format),
                    TextureTypes::Single2D,
                    Texture::Options::kDefaultAspectType,
                    0,
                    1,
                    0,
                    1,
            };

            m_swapChainTextures.Resize(images.size());
            m_imageAvailableSemaphores.Resize(images.size());
            for (u32 i = 0; i < images.size(); i++)
            {
                m_swapChainTextures.Init(i, m_deviceRef, images[i], textureOptions, extent);
                m_imageAvailableSemaphores[i] = m_deviceRef->createSemaphore(vk::SemaphoreCreateInfo{});
            }
        }
    }

    VkSwapChain::~VkSwapChain()
    {
        m_deviceRef->destroy(m_swapChain);
    }

    vk::Semaphore VkSwapChain::AcquireNextImage(u8 _frameIndex)
    {
	    m_imageIndex = m_deviceRef->acquireNextImageKHR(m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[_frameIndex], nullptr).value;
        return m_imageAvailableSemaphores[_frameIndex];
    }

    void VkSwapChain::Present(vk::Queue _presentQueue, const eastl::span<vk::Semaphore>& _semaphores)
    {
	    const vk::PresentInfoKHR presentInfo = {
        	_semaphores,
        	m_swapChain,
            m_imageIndex
        };

        VkAssert(_presentQueue.presentKHR(presentInfo));
    }
}
