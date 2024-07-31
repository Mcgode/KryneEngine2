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
}
