/**
 * @file
 * @author Max Godefroy
 * @date 26/07/2024.
 */

#pragma once

#include <Memory/GenerationalPool.hpp>
#include "Graphics/Common/Enums.hpp"

namespace KryneEngine
{
    struct TextureSrvDesc
    {
        TextureHandle m_texture;

        Texture4ComponentsMapping  m_componentsMapping = KE_DEFAULT_TEXTURE_COMPONENTS_MAPPING;

        u16 m_arrayStart = 0;
        u16 m_arrayRange = 1;

        TextureFormat m_format;

        TextureTypes m_viewType = TextureTypes::Single2D;
        u8 m_minMip = 0;
        u8 m_maxMip = 0;

#if !defined(KE_FINAL)
        eastl::string m_debugName = "";
#endif
    };
}
