/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <EASTL/span.h>
#include <vulkan/vulkan.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Graphics/Common/Enums.hpp>

namespace KryneEngine
{
    template <class Owner, class Resource>
    inline void SafeDestroy(Owner _owner, Resource& _resource)
    {
        _owner.destroy(_resource);
        _resource = nullptr;
    }
}

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
        KE_ASSERT(_api >= GraphicsCommon::Api::Vulkan_Start && _api <= GraphicsCommon::Api::Vulkan_End);
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

#define VkAssert(condition) KE_ASSERT_MSG(vk::Result(condition) == vk::Result::eSuccess, #condition)

    template<class VkType>
    inline bool IsNull(const VkType& _vkObject)
    {
        return static_cast<typename VkType::CType>(_vkObject) == VK_NULL_HANDLE;
    }

    constexpr inline vk::Format ToVkFormat(TextureFormat _format)
    {
        vk::Format format;

        #define MAP(commonFormat, vkFormat) case TextureFormat::commonFormat: format = vk::Format::vkFormat; break

        switch (_format)
        {
            MAP(R8_UNorm, eR8Unorm);
            MAP(RG8_UNorm, eR8G8Unorm);
            MAP(RGB8_UNorm, eR8G8B8Unorm);
            MAP(RGBA8_UNorm, eR8G8B8A8Unorm);

            MAP(RGB8_sRGB, eR8G8B8Srgb);
            MAP(RGBA8_sRGB, eR8G8B8A8Srgb);

            MAP(BGRA8_UNorm, eB8G8R8A8Unorm);
            MAP(BGRA8_sRGB, eB8G8R8A8Srgb);

            MAP(R8_SNorm, eR8Snorm);
            MAP(RG8_SNorm, eR8G8Snorm);
            MAP(RGB8_SNorm, eR8G8B8Snorm);
            MAP(RGBA8_SNorm, eR8G8B8A8Snorm);

            MAP(D16, eD16Unorm);
            MAP(D24, eX8D24UnormPack32);
            MAP(D32F, eD32Sfloat);
            MAP(D24S8, eD24UnormS8Uint);
            MAP(D32FS8, eD32SfloatS8Uint);
            default:
                KE_ASSERT_MSG(_format != TextureFormat::NoFormat, "Unknown format");
                format = vk::Format::eUndefined;
        }

        #undef MAP

        return format;
    }

    constexpr inline TextureFormat FromVkFormat(vk::Format _format)
    {
        TextureFormat format;

        #define MAP(commonFormat, vkFormat) case vk::Format::vkFormat: format = TextureFormat::commonFormat; break

        switch (_format)
        {
            MAP(R8_UNorm, eR8Unorm);
            MAP(RG8_UNorm, eR8G8Unorm);
            MAP(RGB8_UNorm, eR8G8B8Unorm);
            MAP(RGBA8_UNorm, eR8G8B8A8Unorm);

            MAP(RGB8_sRGB, eR8G8B8Srgb);
            MAP(RGBA8_sRGB, eR8G8B8A8Srgb);

            MAP(BGRA8_UNorm, eB8G8R8A8Unorm);
            MAP(BGRA8_sRGB, eB8G8R8A8Srgb);

            MAP(R8_SNorm, eR8Snorm);
            MAP(RG8_SNorm, eR8G8Snorm);
            MAP(RGB8_SNorm, eR8G8B8Snorm);
            MAP(RGBA8_SNorm, eR8G8B8A8Snorm);

            MAP(D16, eD16Unorm);
            MAP(D24, eX8D24UnormPack32);
            MAP(D32F, eD32Sfloat);
            MAP(D24S8, eD24UnormS8Uint);
            MAP(D32FS8, eD32SfloatS8Uint);
            default:
                KE_ASSERT_MSG(_format != vk::Format::eUndefined, "Unknown format");
                format = TextureFormat::NoFormat;
        }

        #undef MAP

        return format;
    }

    constexpr inline vk::ImageViewType RetrieveViewType(TextureTypes _type)
    {
        vk::ImageViewType type;
        switch (_type)
        {
            case TextureTypes::Single1D:
                type = vk::ImageViewType::e1D;
                break;
            case TextureTypes::Single2D:
                type = vk::ImageViewType::e2D;
                break;
            case TextureTypes::Single3D:
                type = vk::ImageViewType::e3D;
                break;
            case TextureTypes::Array1D:
                type = vk::ImageViewType::e1DArray;
                break;
            case TextureTypes::Array2D:
                type = vk::ImageViewType::e2DArray;
                break;
            case TextureTypes::SingleCube:
                type = vk::ImageViewType::eCube;
                break;
            case TextureTypes::ArrayCube:
                type = vk::ImageViewType::eCubeArray;
                break;
            default:
                KE_ERROR("Unknown texture type");
        }
        return type;
    }

    inline vk::ImageAspectFlags RetrieveAspectMask(TexturePlane _plane)
    {
        vk::ImageAspectFlags flags;
        if (BitUtils::EnumHasAny(_plane, TexturePlane::Color))
        {
            flags |= vk::ImageAspectFlagBits::eColor;
        }
        if (BitUtils::EnumHasAny(_plane, TexturePlane::Depth))
        {
            flags |= vk::ImageAspectFlagBits::eDepth;
        }
        if (BitUtils::EnumHasAny(_plane, TexturePlane::Stencil))
        {
            flags |= vk::ImageAspectFlagBits::eStencil;
        }
        return flags;
    }

    constexpr inline vk::ImageLayout ToVkLayout(TextureLayout _layout)
    {
        vk::ImageLayout layout;

        #define MAP(commonLayout, vkLayout) case TextureLayout::commonLayout: layout = vk::ImageLayout::vkLayout; break
        switch (_layout)
        {
            MAP(Unknown, eUndefined);
            MAP(Common, eGeneral);
            MAP(Present, ePresentSrcKHR);
            MAP(GenericRead, eReadOnlyOptimal);
            MAP(ColorAttachment, eColorAttachmentOptimal);
            MAP(DepthStencilAttachment, eDepthStencilAttachmentOptimal);
            MAP(DepthStencilReadOnly, eDepthStencilReadOnlyOptimal);
            MAP(UnorderedAccess, eGeneral); // No specific layout for unordered access resources in VK
            MAP(ShaderResource, eShaderReadOnlyOptimal);
            MAP(TransferSrc, eTransferSrcOptimal);
            MAP(TransferDst, eTransferDstOptimal);
        }
        #undef MAP

        return layout;
    }
}