/**
 * @file
 * @author Max Godefroy
 * @date 26/08/2024.
 */

#include "Helpers.hpp"

#include <Graphics/Common/GraphicsContext.hpp>
#include <runtime/common/rps_render_graph.hpp>

namespace KryneEngine::Modules::RpsRuntime
{
    template <class KeHandle, class RpsHandle>
    KeHandle ToKeHandle(RpsHandle _handle)
    {
        const u32 rawHandle = reinterpret_cast<u64>(_handle.ptr);
        return KeHandle { GenPool::Handle::FromU32(rawHandle) };
    }

    template <class KeHandle, class RpsHandle>
    RpsHandle ToRpsHandle(KeHandle _handle)
    {
        const u64 rawHandle = static_cast<u32>(_handle.m_handle);
        return { reinterpret_cast<void*>(rawHandle) };
    }

#define KE_RPS_IMPLEMENT_CONVERSIONS(KeHandle, RpsHandle)                   \
    template KeHandle ToKeHandle<KeHandle, RpsHandle>(RpsHandle);           \
    template RpsHandle ToRpsHandle<KeHandle, RpsHandle>(KeHandle)

    KE_RPS_IMPLEMENT_CONVERSIONS(TextureHandle, RpsRuntimeResource);
    KE_RPS_IMPLEMENT_CONVERSIONS(BufferHandle, RpsRuntimeResource);

    TexturePlane GetAspectMaskFromFormat(RpsFormat _format)
    {
        switch (_format)
        {
        case RPS_FORMAT_D16_UNORM:
        case RPS_FORMAT_D32_FLOAT:
        case RPS_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case RPS_FORMAT_R24_UNORM_X8_TYPELESS:
            return TexturePlane::Depth;
        case RPS_FORMAT_X24_TYPELESS_G8_UINT:
        case RPS_FORMAT_X32_TYPELESS_G8X24_UINT:
            return TexturePlane::Stencil;
        case RPS_FORMAT_D24_UNORM_S8_UINT:
        case RPS_FORMAT_D32_FLOAT_S8X24_UINT:
        case RPS_FORMAT_R24G8_TYPELESS:
        case RPS_FORMAT_R32G8X24_TYPELESS:
            return TexturePlane::Depth | TexturePlane::Stencil;
        default:
            return TexturePlane::Color;
        }
    }

    MemoryUsage ToKeBufferMemoryUsage(RpsAccessFlags accessFlags)
    {
        MemoryUsage memoryUsage;
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_INDIRECT_ARGS_BIT))
        {
            memoryUsage |= MemoryUsage::IndirectBuffer;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_INDEX_BUFFER_BIT))
        {
            memoryUsage |= MemoryUsage::IndexBuffer;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_VERTEX_BUFFER_BIT))
        {
            memoryUsage |= MemoryUsage::VertexBuffer;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_CONSTANT_BUFFER_BIT))
        {
            memoryUsage |= MemoryUsage::ConstantBuffer;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_SHADER_RESOURCE_BIT))
        {
            memoryUsage |= MemoryUsage::ReadBuffer;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_UNORDERED_ACCESS_BIT))
        {
            memoryUsage |= MemoryUsage::WriteBuffer;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_COPY_SRC_BIT))
        {
            memoryUsage |= MemoryUsage::TransferSrcBuffer;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_COPY_DEST_BIT))
        {
            memoryUsage |= MemoryUsage::TransferDstBuffer;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_RAYTRACING_AS_READ_BIT | RPS_ACCESS_RAYTRACING_AS_BUILD_BIT))
        {
            memoryUsage |= MemoryUsage::TransferDstBuffer;
        }
        return memoryUsage;
    }

    MemoryUsage ToKeTextureMemoryUsage(RpsAccessFlags accessFlags)
    {
        MemoryUsage memoryUsage;
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_RENDER_TARGET_BIT))
        {
            memoryUsage |= MemoryUsage::ColorTargetImage;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_DEPTH_STENCIL))
        {
            memoryUsage |= MemoryUsage::DepthStencilTargetImage;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_SHADER_RESOURCE_BIT))
        {
            memoryUsage |= MemoryUsage::ReadImage | MemoryUsage::SampledImage;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_UNORDERED_ACCESS_BIT))
        {
            memoryUsage |= MemoryUsage::WriteImage;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_COPY_SRC_BIT))
        {
            memoryUsage |= MemoryUsage::TransferSrcImage;
        }
        if (rpsAnyBitsSet(accessFlags, RPS_ACCESS_COPY_DEST_BIT))
        {
            memoryUsage |= MemoryUsage::TransferDstImage;
        }
        return memoryUsage;
    }

    MemoryUsage ToKeHeapMemoryType(const rps::ResourceInstance& resourceInfo)
    {
        if (rpsAnyBitsSet(resourceInfo.desc.flags, RPS_RESOURCE_FLAG_PREFER_GPU_LOCAL_CPU_VISIBLE_BIT)
            && rpsAnyBitsSet(resourceInfo.allAccesses.accessFlags, RPS_ACCESS_CPU_WRITE_BIT)
            && !rpsAnyBitsSet(resourceInfo.allAccesses.accessFlags, RPS_ACCESS_CPU_READ_BIT))
        {
            return MemoryUsage::StageEveryFrame_UsageType;
        }
        else if (rpsAnyBitsSet(resourceInfo.allAccesses.accessFlags, RPS_ACCESS_CPU_WRITE_BIT))
        {
            return MemoryUsage::StageOnce_UsageType;
        }
        else if (rpsAnyBitsSet(resourceInfo.allAccesses.accessFlags, RPS_ACCESS_CPU_READ_BIT))
        {
            return MemoryUsage::Readback_UsageType;
        }
        else
        {
            return MemoryUsage::GpuOnly_UsageType;
        }
    }

    TextureFormat ToKeTextureFormat(RpsFormat format)
    {
        switch (format)
        {
        case RPS_FORMAT_R8_UNORM: return TextureFormat::R8_UNorm;
        case RPS_FORMAT_R8G8_UNORM: return TextureFormat::RG8_UNorm;
        case RPS_FORMAT_R8G8_B8G8_UNORM: return TextureFormat::RGB8_UNorm;
        case RPS_FORMAT_R8G8B8A8_UNORM: return TextureFormat::RGBA8_UNorm;
        case RPS_FORMAT_R8G8B8A8_UNORM_SRGB: return TextureFormat::RGBA8_sRGB;
        case RPS_FORMAT_B8G8R8A8_UNORM: return TextureFormat::BGRA8_UNorm;
        case RPS_FORMAT_B8G8R8A8_UNORM_SRGB: return TextureFormat::BGRA8_sRGB;
        case RPS_FORMAT_R8_SNORM: return TextureFormat::R8_SNorm;
        case RPS_FORMAT_R8G8_SNORM: return TextureFormat::RG8_SNorm;
        case RPS_FORMAT_R8G8B8A8_SNORM: return TextureFormat::RGBA8_SNorm;
        case RPS_FORMAT_R32_FLOAT: return TextureFormat::R32_Float;
        case RPS_FORMAT_R32G32_FLOAT: return TextureFormat::RG32_Float;
        case RPS_FORMAT_R32G32B32_FLOAT: return TextureFormat::RGB32_Float;
        case RPS_FORMAT_R32G32B32A32_FLOAT: return TextureFormat::RGBA32_Float;
        case RPS_FORMAT_D16_UNORM: return TextureFormat::D16;
        case RPS_FORMAT_R24_UNORM_X8_TYPELESS: return TextureFormat::D24;
        case RPS_FORMAT_D24_UNORM_S8_UINT: return TextureFormat::D24S8;
        case RPS_FORMAT_D32_FLOAT: return TextureFormat::D32F;
        case RPS_FORMAT_D32_FLOAT_S8X24_UINT: return TextureFormat::D32FS8;
        default:
            KE_ASSERT_MSG(format == RPS_FORMAT_UNKNOWN, "Unsupported format");
            return TextureFormat::NoFormat;
        }
    }

    RpsFormat ToRpsFormat(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::NoFormat: return RPS_FORMAT_UNKNOWN;
        case TextureFormat::R8_UNorm: return RPS_FORMAT_R8_UNORM;
        case TextureFormat::RG8_UNorm: return RPS_FORMAT_R8G8_UNORM;
        case TextureFormat::RGB8_UNorm: return RPS_FORMAT_R8G8_B8G8_UNORM;
        case TextureFormat::RGBA8_UNorm: return RPS_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGB8_sRGB:
        case TextureFormat::RGBA8_sRGB: return RPS_FORMAT_R8G8B8A8_UNORM_SRGB;
        case TextureFormat::BGRA8_UNorm: return RPS_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::BGRA8_sRGB: return RPS_FORMAT_B8G8R8A8_UNORM_SRGB;
        case TextureFormat::R8_SNorm: return RPS_FORMAT_R8_SNORM;
        case TextureFormat::RG8_SNorm: return RPS_FORMAT_R8G8_SNORM;
        case TextureFormat::RGB8_SNorm:
        case TextureFormat::RGBA8_SNorm: return RPS_FORMAT_R8G8B8A8_SNORM;
        case TextureFormat::R32_Float: return RPS_FORMAT_R32_FLOAT;
        case TextureFormat::RG32_Float: return RPS_FORMAT_R32G32_FLOAT;
        case TextureFormat::RGB32_Float: return RPS_FORMAT_R32G32B32_FLOAT;
        case TextureFormat::RGBA32_Float: return RPS_FORMAT_R32G32B32A32_FLOAT;
        case TextureFormat::D16: return RPS_FORMAT_D16_UNORM;
        case TextureFormat::D24: return RPS_FORMAT_R24_UNORM_X8_TYPELESS;
        case TextureFormat::D24S8: return RPS_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32F: return RPS_FORMAT_D32_FLOAT;
        case TextureFormat::D32FS8: return RPS_FORMAT_D32_FLOAT_S8X24_UINT;
        }
    }
}
