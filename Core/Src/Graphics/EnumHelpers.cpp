/**
 * @file
 * @author Max Godefroy
 * @date 18/04/2022.
 */

#include "KryneEngine/Core/Graphics/EnumHelpers.hpp"
#include "KryneEngine/Core/Common/Assert.hpp"

namespace KryneEngine
{

    u8 GraphicsEnumHelpers::GetTextureFormatComponentCount(TextureFormat _format)
    {
        static_assert(static_cast<u32>(TextureFormat::Count) == 30, "Enum values changed, please update");

        switch (_format)
        {
        case TextureFormat::R8_UNorm:
        case TextureFormat::R8_SNorm:
        case TextureFormat::R16_Float:
        case TextureFormat::R32_Float:
        case TextureFormat::R32_UInt:
        case TextureFormat::D16:
        case TextureFormat::D24:
        case TextureFormat::D32F:
        {
            return 1;
        }

        case TextureFormat::RG8_UNorm:
        case TextureFormat::RG8_SNorm:
        case TextureFormat::RG16_Float:
        case TextureFormat::RG32_Float:
        case TextureFormat::RG32_UInt:
        case TextureFormat::D24S8:
        case TextureFormat::D32FS8:
        {
            return 2;
        }

        case TextureFormat::RGB8_UNorm:
        case TextureFormat::RGB8_sRGB:
        case TextureFormat::RGB8_SNorm:
        case TextureFormat::RGB16_Float:
        case TextureFormat::RGB32_Float:
        case TextureFormat::RGB32_UInt:
        {
            return 3;
        }

        case TextureFormat::RGBA8_UNorm:
        case TextureFormat::RGBA8_sRGB:
        case TextureFormat::BGRA8_UNorm:
        case TextureFormat::BGRA8_sRGB:
        case TextureFormat::RGBA8_SNorm:
        case TextureFormat::RGBA16_Float:
        case TextureFormat::RGBA32_Float:
        case TextureFormat::RGBA32_UInt:
        {
            return 4;
        }

        case TextureFormat::NoFormat:
        case TextureFormat::Count:
        {
            KE_ERROR("No format set");
            return 0;
        }
        }

        KE_ERROR("Format component count not set");
        return 0;
    }

    bool GraphicsEnumHelpers::IsDepthOrStencilFormat(TextureFormat _format)
    {
        switch (_format)
        {
        case TextureFormat::D16:
        case TextureFormat::D24:
        case TextureFormat::D24S8:
        case TextureFormat::D32F:
        case TextureFormat::D32FS8:
            return true;
        default:
            return false;
        }
    }
}
