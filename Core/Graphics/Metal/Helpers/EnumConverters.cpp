/**
 * @file
 * @author Max Godefroy
 * @date 02/11/2024.
 */

#include "EnumConverters.hpp"

namespace KryneEngine::MetalConverters
{
    size_t GetPixelByteSize(TextureFormat _format)
    {
        static_assert(static_cast<u32>(TextureFormat::D32FS8) == 21, "Enum values changed, please update");

        switch (_format)
        {
        case TextureFormat::NoFormat:
        case TextureFormat::D24:
        case TextureFormat::RGB32_Float:
            return 0;
        case TextureFormat::R8_UNorm:
        case TextureFormat::R8_SNorm:
            return 1;
        case TextureFormat::RG8_UNorm:
        case TextureFormat::RG8_SNorm:
        case TextureFormat::D16:
            return 2;
        case TextureFormat::RGB8_UNorm:
        case TextureFormat::RGB8_sRGB:
        case TextureFormat::RGB8_SNorm:
            return 3;
        case TextureFormat::RGBA8_UNorm:
        case TextureFormat::RGBA8_sRGB:
        case TextureFormat::BGRA8_UNorm:
        case TextureFormat::BGRA8_sRGB:
        case TextureFormat::RGBA8_SNorm:
        case TextureFormat::R32_Float:
        case TextureFormat::D24S8:
        case TextureFormat::D32F:
            return 4;
        case TextureFormat::RG32_Float:
        case TextureFormat::D32FS8:
            return 8;
        case TextureFormat::RGBA32_Float:
            return 16;
        }
    }

    MTL::PixelFormat ToPixelFormat(TextureFormat _format)
    {
        static_assert(static_cast<u32>(TextureFormat::D32FS8) == 21, "Enum values changed, please update");

        switch (_format)
        {
        case TextureFormat::NoFormat:
        case TextureFormat::RGB8_UNorm:
        case TextureFormat::RGB8_sRGB:
        case TextureFormat::RGB8_SNorm:
        case TextureFormat::RGB32_Float:
        case TextureFormat::D24:
            KE_ASSERT_FATAL_MSG(_format == TextureFormat::NoFormat, "Unsupported format");
            return MTL::PixelFormatInvalid;
        case TextureFormat::R8_UNorm:
            return MTL::PixelFormatR8Unorm;
        case TextureFormat::RG8_UNorm:
            return MTL::PixelFormatRG8Unorm;
        case TextureFormat::RGBA8_UNorm:
            return MTL::PixelFormatRGBA8Unorm;
        case TextureFormat::RGBA8_sRGB:
            return MTL::PixelFormatRGBA8Unorm_sRGB;
        case TextureFormat::BGRA8_UNorm:
            return MTL::PixelFormatBGRA8Unorm;
        case TextureFormat::BGRA8_sRGB:
            return MTL::PixelFormatBGRA8Unorm_sRGB;
        case TextureFormat::R8_SNorm:
            return MTL::PixelFormatR8Snorm;
        case TextureFormat::RG8_SNorm:
            return MTL::PixelFormatRG8Snorm;
        case TextureFormat::RGBA8_SNorm:
            return MTL::PixelFormatRGBA8Snorm;
        case TextureFormat::R32_Float:
            return MTL::PixelFormatR32Float;
        case TextureFormat::RG32_Float:
            return MTL::PixelFormatRG32Float;
        case TextureFormat::RGBA32_Float:
            return MTL::PixelFormatRGBA32Float;
        case TextureFormat::D16:
            return MTL::PixelFormatDepth16Unorm;
        case TextureFormat::D24S8:
            return MTL::PixelFormatDepth24Unorm_Stencil8;
        case TextureFormat::D32F:
            return MTL::PixelFormatDepth32Float;
        case TextureFormat::D32FS8:
            return MTL::PixelFormatDepth32Float_Stencil8;
        }
    }

    MTL::ResourceOptions GetResourceStorage(MemoryUsage _memoryUsage)
    {
        switch (_memoryUsage & MemoryUsage::USAGE_TYPE_MASK)
        {
        case MemoryUsage::StageOnce_UsageType:
            return MTL::ResourceStorageModeShared;
        case MemoryUsage::StageEveryFrame_UsageType:
#if defined(TARGET_OS_MAC)
            return MTL::ResourceStorageModeManaged;
#else
            return MTL::ResourceStorageModeShared;
#endif
        case MemoryUsage::GpuOnly_UsageType:
            return MTL::ResourceStorageModePrivate;
        case MemoryUsage::Readback_UsageType:
#if defined(TARGET_OS_MAC)
            return MTL::ResourceStorageModeManaged;
#else
            return MTL::ResourceStorageModeShared;
#endif
        }
        return 0;
    }

    MTL::StorageMode GetStorageMode(MemoryUsage _memoryUsage)
    {
        switch (_memoryUsage & MemoryUsage::USAGE_TYPE_MASK)
        {
        case MemoryUsage::StageOnce_UsageType:
            return MTL::StorageModeShared;
        case MemoryUsage::StageEveryFrame_UsageType:
#if defined(TARGET_OS_MAC)
            return MTL::StorageModeManaged;
#else
            return MTL::StorageModeShared;
#endif
        case MemoryUsage::GpuOnly_UsageType:
            return MTL::StorageModePrivate;
        case MemoryUsage::Readback_UsageType:
#if defined(TARGET_OS_MAC)
            return MTL::StorageModeManaged;
#else
            return MTL::StorageModeShared;
#endif
        default:
            return MTL::StorageModeShared;
        }
    }

    MTL::TextureSwizzle GetSwizzle(TextureComponentMapping _mapping)
    {
        switch (_mapping)
        {
        case TextureComponentMapping::Red:
            return MTL::TextureSwizzleRed;
        case TextureComponentMapping::Green:
            return MTL::TextureSwizzleGreen;
        case TextureComponentMapping::Blue:
            return MTL::TextureSwizzleBlue;
        case TextureComponentMapping::Alpha:
            return MTL::TextureSwizzleAlpha;
        case TextureComponentMapping::Zero:
            return MTL::TextureSwizzleZero;
        case TextureComponentMapping::One:
            return MTL::TextureSwizzleOne;
        }
    }

    MTL::TextureType GetTextureType(TextureTypes _type)
    {
        switch (_type)
        {
        case TextureTypes::Single1D:
            return MTL::TextureType1D;
        case TextureTypes::Single2D:
            return MTL::TextureType2D;
        case TextureTypes::Single3D:
            return MTL::TextureType3D;
        case TextureTypes::Array1D:
            return MTL::TextureType1DArray;
        case TextureTypes::Array2D:
            return MTL::TextureType2DArray;
        case TextureTypes::SingleCube:
            return MTL::TextureTypeCube;
        case TextureTypes::ArrayCube:
            return MTL::TextureTypeCubeArray;
        }
    }

    MTL::TextureUsage GetTextureUsage(MemoryUsage _usage)
    {
        MTL::TextureUsage usage {};
        if (BitUtils::EnumHasAny(_usage, MemoryUsage::ReadImage))
        {
            usage |= MTL::TextureUsageShaderRead;
        }
        if (BitUtils::EnumHasAny(_usage, MemoryUsage::WriteImage))
        {
            usage |= MTL::TextureUsageShaderWrite;
        }
        if (BitUtils::EnumHasAny(_usage, MemoryUsage::ColorTargetImage | MemoryUsage::DepthStencilTargetImage))
        {
            usage |= MTL::TextureUsageRenderTarget;
        }
        return usage;
    }

    MTL::LoadAction GetMetalLoadOperation(RenderPassDesc::Attachment::LoadOperation _op)
    {
        switch (_op)
        {
        case RenderPassDesc::Attachment::LoadOperation::Load:
            return MTL::LoadActionLoad;
        case RenderPassDesc::Attachment::LoadOperation::Clear:
            return MTL::LoadActionClear;
        case RenderPassDesc::Attachment::LoadOperation::DontCare:
            return MTL::LoadActionDontCare;
        }
    }

    MTL::StoreAction GetMetalStoreOperation(RenderPassDesc::Attachment::StoreOperation _op)
    {
        switch (_op)
        {
        case RenderPassDesc::Attachment::StoreOperation::Store:
            return MTL::StoreActionStore;
        case RenderPassDesc::Attachment::StoreOperation::Resolve:
            return MTL::StoreActionStoreAndMultisampleResolve;
        case RenderPassDesc::Attachment::StoreOperation::DontCare:
            return MTL::StoreActionDontCare;
        }
    }
}