/**
 * @file
 * @author Max Godefroy
 * @date 10/07/2022.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <EASTL/optional.h>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/VK/CommonStructures.hpp>

namespace KryneEngine
{
    class VkTexture: public Texture
    {
        friend class VkGraphicContext;

    public:
        /// @brief Used to reference system images. (ex: swapchain images)
        VkTexture(const VkSharedDeviceRef &_device, const vk::Image &_image,
                  const Options &_textureOptions, const vk::Extent2D &_imageSize);

        ~VkTexture() override;

        [[nodiscard]] bool IsRef() const override
        {
            return !m_image.has_value();
        }

    private:
        VkSharedDeviceRef m_device;
        eastl::optional<VkSharedImage> m_image {};
        VkSharedImageRef m_imageRef {};
        vk::ImageView m_imageView = nullptr;

        void _CreateImageView(const vk::Image& _image, const Options& _options);
    };
} // KryneEngine