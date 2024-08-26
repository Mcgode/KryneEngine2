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
}
