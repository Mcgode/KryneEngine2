/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#pragma once

#include <Common/KETypes.hpp>

namespace KryneEngine
{
    enum class TextureFormat : u32
    {
        NoFormat,

        R8_UNorm,
        RG8_UNorm,
        RGB8_UNorm,
        RGBA8_UNorm,

        RGB8_UNorm_sRGB,
        RGBA8_UNorm_sRGB,

        R8_SNorm,
        RG8_SNorm,
        RGB8_SNorm,
        RGBA8_SNorm,

        D16,
        D24,
        D24S8,
        D32F,
        D32FS8,
    };
}