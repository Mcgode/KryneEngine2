/**
 * @file
 * @author Max Godefroy
 * @date 26/08/2024.
 */

#pragma once

#include <Graphics/Common/Enums.hpp>
#include <rps/runtime/common/rps_access.h>
#include <rps/runtime/common/rps_format.h>

namespace rps
{
    struct ResourceInstance;
}

namespace KryneEngine::Modules::RpsRuntime
{
    template <class KeHandle, class RpsHandle>
    KeHandle ToKeHandle(RpsHandle _handle);

    template <class KeHandle, class RpsHandle>
    RpsHandle ToRpsHandle(KeHandle _handle);

    [[nodiscard]] TexturePlane GetAspectMaskFromFormat(RpsFormat _format);

    [[nodiscard]] MemoryUsage ToKeBufferMemoryUsage(RpsAccessFlags accessFlags);
    [[nodiscard]] MemoryUsage ToKeTextureMemoryUsage(RpsAccessFlags accessFlags);
    [[nodiscard]] MemoryUsage ToKeHeapMemoryType(const rps::ResourceInstance& resourceInfo);

    [[nodiscard]] TextureFormat ToKeTextureFormat(RpsFormat format);
    [[nodiscard]] RpsFormat ToRpsFormat(TextureFormat format);
}
