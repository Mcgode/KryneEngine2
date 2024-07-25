/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#pragma once

#include <Common/Types.hpp>
#include <Common/BitUtils.hpp>

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
        BGRA8_UNorm,
        BGRA8_sRGB,

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

    enum class TextureUsage : u8
    {
        TransferSource          = 1 << 0,
        TransferDestination     = 1 << 1,
        ShaderSampling          = 1 << 2,
        UnorderedAccess         = 1 << 3,
        ColorAttachment         = 1 << 4,
        DepthStencilAttachment  = 1 << 5,
    };
    KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(TextureUsage)

    enum class TextureLayout: u8
    {
        Unknown,
        Common,
        Present,
        GenericRead,
        ColorAttachment,
        DepthStencilAttachment,
        DepthStencilReadOnly,
        UnorderedAccess,
        ShaderResource,
        TransferSrc,
        TransferDst,
    };

    enum class ResourceAccess
    {

    };

    enum class TexturePlane: u8
    {
        Color   = 1 << 0,
        Depth   = 1 << 1,
        Stencil = 1 << 2,
    };
    KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(TexturePlane)

    enum class MemoryAccessType: u8
    {
        StageOnce = 0,
        StageEveryFrame,
        GpuOnly,
        Readback,
    };
}