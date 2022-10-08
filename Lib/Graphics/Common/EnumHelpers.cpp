/**
 * @file
 * @author Max Godefroy
 * @date 18/04/2022.
 */

#include <Common/Assert.hpp>
#include "EnumHelpers.hpp"

KryneEngine::u8 KryneEngine::GraphicsEnumHelpers::GetTextureFormatComponentCount(KryneEngine::TextureFormat _format)
{
    switch (_format)
    {
        case TextureFormat::R8_UNorm:
        case TextureFormat::R8_SNorm:
        case TextureFormat::D16:
        case TextureFormat::D24:
        case TextureFormat::D32F:
        {
            return 1;
        }

        case TextureFormat::RG8_UNorm:
        case TextureFormat::RG8_SNorm:
        case TextureFormat::D24S8:
        case TextureFormat::D32FS8:
        {
            return 2;
        }

        case TextureFormat::RGB8_UNorm:
        case TextureFormat::RGB8_UNorm_sRGB:
        case TextureFormat::RGB8_SNorm:
        {
            return 3;
        }

        case TextureFormat::RGBA8_UNorm:
        case TextureFormat::RGBA8_UNorm_sRGB:
        case TextureFormat::RGBA8_SNorm:
        {
            return 4;
        }

        case TextureFormat::NoFormat:
        {
            Error("No format set");
            return 0;
        }
    }

    Error("Format component count not set");
    return 0;
}
