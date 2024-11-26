/**
 * @file
 * @author Max Godefroy
 * @date 18/04/2022.
 */

#pragma once

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Graphics/Common/Enums.hpp"

namespace KryneEngine::GraphicsEnumHelpers
{
    u8 GetTextureFormatComponentCount(TextureFormat _format);

    bool IsDepthOrStencilFormat(TextureFormat _format);
}