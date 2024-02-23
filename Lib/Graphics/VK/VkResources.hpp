/**
 * @file
 * @author Max Godefroy
 * @date 19/02/2024.
 */

#pragma once

#include "VkHeaders.hpp"

namespace KryneEngine
{
    struct RenderTargetViewDesc;

    struct VkResources
    {
        GenerationalPool<vk::Image> m_textures {};
        GenerationalPool<vk::ImageView> m_renderTargetViews {};

        // Default constructor and destructor, but moved implementation to cpp for .inl linking
        VkResources();
        ~VkResources();

        [[nodiscard]] GenPool::Handle RegisterTexture(vk::Image _image);

        bool ReleaseTexture(GenPool::Handle _handle, vk::Device _device, bool _free = true);

        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc, vk::Device& _device);

        bool FreeRenderTargetView(GenPool::Handle _handle, vk::Device _device);
    };
} // KryneEngine