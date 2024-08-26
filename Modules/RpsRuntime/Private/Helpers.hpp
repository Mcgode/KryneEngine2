/**
 * @file
 * @author Max Godefroy
 * @date 26/08/2024.
 */

#pragma once

#include <Graphics/Common/Enums.hpp>
#include <rps/runtime/common/rps_format.h>

namespace KryneEngine::Modules::RpsRuntime
{
    template <class KeHandle, class RpsHandle>
    KeHandle ToKeHandle(RpsHandle _handle);

    template <class KeHandle, class RpsHandle>
    RpsHandle ToRpsHandle(KeHandle _handle);

    TexturePlane GetAspectMaskFromFormat(RpsFormat _format);
}
