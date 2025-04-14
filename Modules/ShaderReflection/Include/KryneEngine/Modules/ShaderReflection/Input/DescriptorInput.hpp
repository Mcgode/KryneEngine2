/**
 * @file
 * @author Max Godefroy
 * @date 14/04/2025.
 */

#pragma once

#include <EASTL/string_view.h>
#include <KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp>

namespace KryneEngine::Modules::ShaderReflection
{
    struct DescriptorInput
    {
        eastl::string_view m_name;
        DescriptorBindingDesc::Type m_type;
        u16 m_count;
        u16 m_bindingIndex;
        TextureTypes m_textureType;
        bool m_used;
    };
}