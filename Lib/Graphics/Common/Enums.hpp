/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#pragma once

#include <Common/KETypes.hpp>

namespace KryneEngine
{
    enum class TextureFormat : u16
    {
        NoFormat,

        R8_UNorm,
        RG8_UNorm,
        RGB8_UNorm,
        RGBA8_UNorm,

        RGB8_sRGB,
        RGBA8_sRGB,

        // Present special formats
        BRGA8_UNorm,
        BRGA8_sRGB,

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

    enum class TextureTypes: u8
    {
        Single1D,
        Single2D,
        Single3D,
        Array1D,
        Array2D,
        SingleCube,
        ArrayCube,
    };

    enum class TextureAspectType: u8
    {
        Color,
        Depth,
        Stencil,
    };
}