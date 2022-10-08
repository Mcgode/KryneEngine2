/**
 * @file
 * @author Max Godefroy
 * @date 10/07/2022.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/VK/CommonStructures.hpp>

namespace KryneEngine
{
    class VkTexture: public Texture
    {
        friend class VkGraphicContext;

    public:
        ~VkTexture() override;

        [[nodiscard]] bool IsRef() const override
        {
            return !m_image;
        }

    private:
        explicit VkTexture(VkSharedDeviceRef&& _device)
            : m_device(eastl::move(_device))
        {}

        VkSharedDeviceRef m_device;
        vk::Image m_image = nullptr;
        vk::ImageView m_imageView = nullptr;
    };
} // KryneEngine