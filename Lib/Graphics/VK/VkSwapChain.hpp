/**
 * @file
 * @author Max Godefroy
 * @date 20/03/2022.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <Common/KETypes.hpp>

namespace KryneEngine
{
    class VkSwapChain
    {
    public:
        VkSwapChain(const vk::PhysicalDevice& _physicalDevice, const vk::SurfaceKHR& _surface);

    private:
        vk::SwapchainKHR m_swapChain;
    };
}
