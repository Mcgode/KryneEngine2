/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include "Common/Assert.hpp"

namespace KryneEngine::VkHelperFunctions
{
    inline u32 MakeVersion(const GraphicsCommon::Version& _version)
    {
        return VK_MAKE_VERSION(
            _version.m_major,
            _version.m_minor,
            _version.m_revision
        );
    }

    inline u32 GetApiVersion(GraphicsCommon::Api _api)
    {
        Assert(_api >= GraphicsCommon::Api::VulkanStart && _api <= GraphicsCommon::Api::VulkanEnd);
        switch (_api)
        {
            case GraphicsCommon::Api::Vulkan_1_1: return VK_API_VERSION_1_1;
            case GraphicsCommon::Api::Vulkan_1_2: return VK_API_VERSION_1_2;
            case GraphicsCommon::Api::Vulkan_1_3: return VK_API_VERSION_1_2;
            default: return VK_API_VERSION_1_0;
        }
    }

    template<class Container>
    inline vk::ArrayProxyNoTemporaries<const typename Container::value_type> MakeArrayProxy(const Container& _container)
    {
        return vk::ArrayProxyNoTemporaries<const typename Container::value_type>(_container.size(), _container.data());
    }

    inline void VkAssert(vk::Result _result)
    {
        Assert(_result == vk::Result::eSuccess);
    }

    inline void VkAssert(VkResult _vkResult)
    {
        VkAssert(vk::Result(_vkResult));
    }
}