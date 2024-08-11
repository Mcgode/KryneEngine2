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

        TexturePlane m_planes = TexturePlane::Color;

#if !defined(KE_FINAL)
        eastl::string m_debugName;
#endif
    };

    struct TextureMemoryFootprint
    {
        u64 m_offset;

        u32 m_width;
        u32 m_height;

        u32 m_lineByteAlignedSize;
        u16 m_depth;
        TextureFormat m_format;
    };

    struct TextureCreateDesc
    {
        TextureDesc m_desc;
        eastl::vector<TextureMemoryFootprint> m_footprintPerSubResource {};
        MemoryUsage m_memoryUsage = MemoryUsage::Invalid;
    };

    struct SubResourceIndexing
    {
        u16 m_arraySize;
        u16 m_arraySlice;

        u8 m_mipCount;
        u8 m_mipIndex;

        TexturePlane m_planes;
        TexturePlane m_planeSlice;

        SubResourceIndexing(const TextureDesc& _desc, u8 _mipIndex, u16 _arraySlice = 0, TexturePlane _planeSlice = TexturePlane::Color)
            : m_arraySize(_desc.m_arraySize)
            , m_arraySlice(_arraySlice)
            , m_mipCount(_desc.m_mipCount)
            , m_mipIndex(_mipIndex)
            , m_planes(_desc.m_planes)
            , m_planeSlice(_planeSlice)
        {}
    };

    struct SamplerDesc
    {
        enum class Filter: u8
        {
            Point,
            Linear,
        };

        enum class AddressMode: u8
        {
            Repeat,
            MirroredRepeat,
            Border,
            Clamp,
        };

        enum class OpType: u8
        {
            Blend,
            Minimum,
            Maximum,
        };

        Filter m_minFilter = Filter::Linear;
        Filter m_magFilter = Filter::Linear;
        Filter m_mipFilter = Filter::Linear;
        AddressMode m_addressModeU = AddressMode::Repeat;
        AddressMode m_addressModeV = AddressMode::Repeat;
        AddressMode m_addressModeW = AddressMode::Repeat;

        OpType m_opType = OpType::Blend;
        u8 m_anisotropy = 0;

        float4 m_borderColor = { 0, 0, 0, 1 };

        float m_lodBias = 0.f;
        float m_lodMin = 0.f;
        float m_lodMax = kNoMaxLod;
        static constexpr float kNoMaxLod = 1024.f;
    };
}
