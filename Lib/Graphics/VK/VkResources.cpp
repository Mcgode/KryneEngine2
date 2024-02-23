/**
 * @file
 * @author Max Godefroy
 * @date 19/02/2024.
 */

#include "VkResources.hpp"
#include "HelperFunctions.hpp"
#include <Graphics/Common/RenderTargetView.hpp>
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    VkResources::VkResources()  = default;
    VkResources::~VkResources() = default;

    GenPool::Handle VkResources::RegisterTexture(const vk::Image _image)
    {
        const auto handle = m_textures.Allocate();
        *m_textures.Get(handle) = _image;
        return handle;
    }

    bool VkResources::ReleaseTexture(GenPool::Handle _handle, vk::Device _device, bool _free)
    {
        vk::Image image;
        if (m_textures.Free(_handle, &image))
        {
            if (_free)
            {
                _device.destroy(image);
            }
            return true;
        }
        return false;
    }

    GenPool::Handle VkResources::CreateRenderTargetView(
            const RenderTargetViewDesc &_desc,
            vk::Device &_device)
    {
        auto* image = m_textures.Get(_desc.m_textureHandle);
        if (image == nullptr)
        {
            return GenPool::kInvalidHandle;
        }

        const auto rtvHandle = m_renderTargetViews.Allocate();

        vk::ImageViewCreateInfo imageViewCreateInfo(
                {},
                *image,
                VkHelperFunctions::RetrieveViewType(_desc.m_type),
                VkHelperFunctions::ToVkFormat(_desc.m_format),
                {},
                {
                        VkHelperFunctions::RetrieveAspectMask(_desc.m_plane),
                        _desc.m_mipLevel,
                        1,
                        _desc.m_arrayRangeStart,
                        _desc.m_arrayRangeSize,
                }
        );

        *m_renderTargetViews.Get(rtvHandle) = _device.createImageView(imageViewCreateInfo);

        return rtvHandle;
    }

    bool VkResources::FreeRenderTargetView(GenPool::Handle _handle, vk::Device _device)
    {
        vk::ImageView imageView;
        if (m_renderTargetViews.Free(_handle, &imageView))
        {
            _device.destroy(imageView);
            return true;
        }
        return false;
    }
}
