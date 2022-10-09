/**
 * @file
 * @author Max Godefroy
 * @date 10/07/2022.
 */

#include "VkTexture.hpp"
#include "HelperFunctions.hpp"

namespace KryneEngine
{
    VkTexture::VkTexture(const VkSharedDeviceRef &_device,
                         const vk::Image &_image,
                         const Options &_textureOptions,
                         const vk::Extent2D &_imageSize)
            : m_device(_device)
    {
        m_size.x = _imageSize.width;
        m_size.y = _imageSize.height;

        m_format = _textureOptions.m_format;

        _CreateImageView(_image, _textureOptions);
    }

    VkTexture::~VkTexture()
    {
        // In order:
        // - destroy image view
        // - unref image (if reffed)
        // - destroy image (if owned)
        m_device->destroy(m_imageView);
        m_imageRef.Reset();
        m_image.reset();
    }

    void VkTexture::_CreateImageView(const vk::Image &_image, const Texture::Options &_options)
    {
        Assert(!m_imageView, "Previous image view was not deleted");

        vk::ImageViewCreateInfo createInfo;

        createInfo.image = _image;
        createInfo.viewType = VkHelperFunctions::RetrieveViewType(_options.m_type);
        createInfo.format = VkHelperFunctions::RetrieveFormat(_options.m_format);
        createInfo.components = vk::ComponentMapping();
        createInfo.subresourceRange = vk::ImageSubresourceRange {
                VkHelperFunctions::GetAspectMask(_options.m_textureAspect),
                _options.m_baseMipLevel,
                _options.m_mipLevelCount,
                _options.m_baseArrayLayer,
                _options.m_arrayLayerCount
        };

        const auto result = m_device->createImageView(createInfo);
        Assert(result);
        m_imageView = result;
    }
} // KryneEngine