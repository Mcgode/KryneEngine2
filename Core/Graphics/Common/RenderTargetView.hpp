/**
 * @file
 * @author Max Godefroy
 * @date 14/02/2024.
 */

#pragma once

#include <Memory/GenerationalPool.hpp>
#include "Enums.hpp"

namespace KryneEngine
{
    struct RenderTargetViewDesc
    {
        GenPool::Handle m_textureHandle = GenPool::kInvalidHandle;

        TextureFormat m_format = TextureFormat::NoFormat;
        TextureTypes m_type = TextureTypes::Single2D;
        TexturePlane m_plane = TexturePlane::Color;

        union {
            u16 m_arrayRangeStart = 0;
            u16 m_depthStartSlice;
        };
        union {
            u16 m_arrayRangeSize = 1;
            u16 m_depthSlicesSize;
        };

        u8 m_mipLevel = 0;

#if !defined(KE_FINAL)
        eastl::string m_debugName;
#endif
    };
}
