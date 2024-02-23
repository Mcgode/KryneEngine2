/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#pragma once

#include <EASTL/span.h>
#include <Graphics/Common/Enums.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Common/Types.hpp>

namespace KryneEngine
{
    struct TextureDesc
    {
        uint3 m_dimensions {};

        TextureFormat m_format = TextureFormat::NoFormat;
        u16 m_arraySize = 1;

        TextureTypes m_type = TextureTypes::Single2D;
        u8 m_mipCount = 1;
    };
}
