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
}
