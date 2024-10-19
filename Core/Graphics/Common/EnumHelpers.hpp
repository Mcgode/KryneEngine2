/**
 * @file
 * @author Max Godefroy
 * @date 18/04/2022.
 */

#pragma once

#include <Graphics/Common/Enums.hpp>
#include <Common/Types.hpp>

namespace KryneEngine::GraphicsEnumHelpers
{
    u8 GetTextureFormatComponentCount(TextureFormat _format);

    bool IsDepthOrStencilFormat(TextureFormat _format);
}