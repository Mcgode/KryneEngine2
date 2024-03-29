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
        Assert(_api >= GraphicsCommon::Api::Vulkan_Start && _api <= GraphicsCommon::Api::Vulkan_End);
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

    template<class VkType>
    inline bool IsNull(const VkType& _vkObject)
    {
        return static_cast<typename VkType::CType>(_vkObject) == VK_NULL_HANDLE;
    }

    constexpr inline vk::Format RetrieveFormat(TextureFormat _format)
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

            MAP(BRGA8_UNorm, eB8G8R8A8Unorm);
            MAP(BRGA8_sRGB, eB8G8R8A8Srgb);

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
                Assert(_format != TextureFormat::NoFormat, "Unknown format");
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

            MAP(BRGA8_UNorm, eB8G8R8A8Unorm);
            MAP(BRGA8_sRGB, eB8G8R8A8Srgb);

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
                Assert(_format != vk::Format::eUndefined, "Unknown format");
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
                Error("Unknown texture type");
        }
        return type;
    }

    inline vk::ImageAspectFlags GetAspectMask(const eastl::span<const TextureAspectType>& _span)
    {
        vk::ImageAspectFlags flags;
        for (auto aspect: _span)
        {
            switch (aspect)
            {
                case TextureAspectType::Color:
                    flags |= vk::ImageAspectFlagBits::eColor;
                    break;
                case TextureAspectType::Depth:
                    flags |= vk::ImageAspectFlagBits::eDepth;
                    break;
                case TextureAspectType::Stencil:
                    flags |= vk::ImageAspectFlagBits::eStencil;
                    break;
                default:
                    Error("Unknown aspect type");
            }
        }
        return flags;
    }
}