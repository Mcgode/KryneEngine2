/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <EASTL/span.h>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Graphics/Common/Enums.hpp>

namespace KryneEngine
{
    template <class Resource>
    inline Resource SafeReset(Resource& _resource)
    {
        const Resource value = _resource;
        _resource = VK_NULL_HANDLE;
        return value;
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

#define VkAssert(condition) KE_ASSERT_MSG(VkResult(condition) == VK_SUCCESS, #condition)

    template<class VkType>
    inline bool IsNull(const VkType& _vkObject)
    {
        return static_cast<typename VkType::CType>(_vkObject) == VK_NULL_HANDLE;
    }

    template <class T, class FunctionReturnT, class... FunctionArgsT, class... Args>
    inline void VkArrayFetch(DynamicArray<T>& _array, FunctionReturnT (*_fetchFunction)(FunctionArgsT...), Args&&... _args)
    {
        constexpr bool returnsVoid = eastl::is_same<FunctionReturnT, void>::value;

        u32 count;
        if constexpr (returnsVoid)
        {
            _fetchFunction(eastl::forward<Args>(_args)..., &count, nullptr);
        }
        else
        {
            VkAssert(_fetchFunction(eastl::forward<Args>(_args)..., &count, nullptr));
        }

        _array.Resize(count);
        if constexpr (returnsVoid)
        {
            _fetchFunction(eastl::forward<Args>(_args)..., &count, _array.Data());
        }
        else
        {
            VkAssert(_fetchFunction(eastl::forward<Args>(_args)..., &count, _array.Data()));
        }
    }

    constexpr inline VkFormat ToVkFormat(TextureFormat _format)
    {
        VkFormat format;

        #define MAP(commonFormat, vkFormat) case TextureFormat::commonFormat: format = vkFormat; break

        switch (_format)
        {
            MAP(R8_UNorm, VK_FORMAT_R8_UNORM);
            MAP(RG8_UNorm, VK_FORMAT_R8G8_UNORM);
            MAP(RGB8_UNorm, VK_FORMAT_R8G8B8_UNORM);
            MAP(RGBA8_UNorm, VK_FORMAT_R8G8B8A8_UNORM);

            MAP(RGB8_sRGB, VK_FORMAT_R8G8B8_SRGB);
            MAP(RGBA8_sRGB, VK_FORMAT_R8G8B8A8_SRGB);

            MAP(BGRA8_UNorm, VK_FORMAT_B8G8R8A8_UNORM);
            MAP(BGRA8_sRGB, VK_FORMAT_B8G8R8A8_SRGB);

            MAP(R8_SNorm, VK_FORMAT_R8_SNORM);
            MAP(RG8_SNorm, VK_FORMAT_R8G8_SNORM);
            MAP(RGB8_SNorm, VK_FORMAT_R8G8B8_SNORM);
            MAP(RGBA8_SNorm, VK_FORMAT_R8G8B8A8_SNORM);

            MAP(D16, VK_FORMAT_D16_UNORM);
            MAP(D24, VK_FORMAT_X8_D24_UNORM_PACK32);
            MAP(D32F, VK_FORMAT_D32_SFLOAT);
            MAP(D24S8, VK_FORMAT_D24_UNORM_S8_UINT);
            MAP(D32FS8, VK_FORMAT_D32_SFLOAT_S8_UINT);
            default:
                KE_ASSERT_MSG(_format != TextureFormat::NoFormat, "Unknown format");
                format = VK_FORMAT_UNDEFINED;
        }

        #undef MAP

        return format;
    }

    constexpr inline TextureFormat FromVkFormat(VkFormat _format)
    {
        TextureFormat format;

        #define MAP(commonFormat, vkFormat) case vkFormat: format = TextureFormat::commonFormat; break

        switch (_format)
        {
            MAP(R8_UNorm, VK_FORMAT_R8_UNORM);
            MAP(RG8_UNorm, VK_FORMAT_R8G8_UNORM);
            MAP(RGB8_UNorm, VK_FORMAT_R8G8B8_UNORM);
            MAP(RGBA8_UNorm, VK_FORMAT_R8G8B8A8_UNORM);

            MAP(RGB8_sRGB, VK_FORMAT_R8G8B8_SRGB);
            MAP(RGBA8_sRGB, VK_FORMAT_R8G8B8A8_SRGB);

            MAP(BGRA8_UNorm, VK_FORMAT_B8G8R8A8_UNORM);
            MAP(BGRA8_sRGB, VK_FORMAT_B8G8R8A8_SRGB);

            MAP(R8_SNorm, VK_FORMAT_R8_SNORM);
            MAP(RG8_SNorm, VK_FORMAT_R8G8_SNORM);
            MAP(RGB8_SNorm, VK_FORMAT_R8G8B8_SNORM);
            MAP(RGBA8_SNorm, VK_FORMAT_R8G8B8A8_SNORM);

            MAP(D16, VK_FORMAT_D16_UNORM);
            MAP(D24, VK_FORMAT_X8_D24_UNORM_PACK32);
            MAP(D32F, VK_FORMAT_D32_SFLOAT);
            MAP(D24S8, VK_FORMAT_D24_UNORM_S8_UINT);
            MAP(D32FS8, VK_FORMAT_D32_SFLOAT_S8_UINT);
            default:
                KE_ASSERT_MSG(_format != VK_FORMAT_UNDEFINED, "Unknown format");
                format = TextureFormat::NoFormat;
        }

        #undef MAP

        return format;
    }

    constexpr inline VkImageViewType RetrieveViewType(TextureTypes _type)
    {
        VkImageViewType type;
        switch (_type)
        {
            case TextureTypes::Single1D:
                type = VK_IMAGE_VIEW_TYPE_1D;
                break;
            case TextureTypes::Single2D:
                type = VK_IMAGE_VIEW_TYPE_2D;
                break;
            case TextureTypes::Single3D:
                type = VK_IMAGE_VIEW_TYPE_3D;
                break;
            case TextureTypes::Array1D:
                type = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                break;
            case TextureTypes::Array2D:
                type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                break;
            case TextureTypes::SingleCube:
                type = VK_IMAGE_VIEW_TYPE_CUBE;
                break;
            case TextureTypes::ArrayCube:
                type = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                break;
            default:
                KE_ERROR("Unknown texture type");
        }
        return type;
    }

    inline u32 RetrieveAspectMask(TexturePlane _plane)
    {
        u32 flags = 0;
        if (BitUtils::EnumHasAny(_plane, TexturePlane::Color))
        {
            flags |= VK_IMAGE_ASPECT_COLOR_BIT;
        }
        if (BitUtils::EnumHasAny(_plane, TexturePlane::Depth))
        {
            flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (BitUtils::EnumHasAny(_plane, TexturePlane::Stencil))
        {
            flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        return flags;
    }

    constexpr inline VkImageLayout ToVkLayout(TextureLayout _layout)
    {
        VkImageLayout layout;

        #define MAP(commonLayout, vkLayout) case TextureLayout::commonLayout: layout = vkLayout; break
        switch (_layout)
        {
            MAP(Unknown, VK_IMAGE_LAYOUT_UNDEFINED);
            MAP(Common, VK_IMAGE_LAYOUT_GENERAL);
            MAP(Present, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            MAP(GenericRead, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);
            MAP(ColorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            MAP(DepthStencilAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            MAP(DepthStencilReadOnly, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
            MAP(UnorderedAccess, VK_IMAGE_LAYOUT_GENERAL); // No specific layout for unordered access resources in VK
            MAP(ShaderResource, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            MAP(TransferSrc, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            MAP(TransferDst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        }
        #undef MAP

        return layout;
    }

    constexpr inline VkDebugReportObjectTypeEXT ConvertObjectType(VkObjectType _objectType)
    {
        if (_objectType <= VK_OBJECT_TYPE_COMMAND_POOL)
        {
            return static_cast<VkDebugReportObjectTypeEXT>(_objectType);
        }
        return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    }
}