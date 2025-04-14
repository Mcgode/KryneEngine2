/**
 * @file
 * @author Max Godefroy
 * @date 14/04/2025.
 */

#pragma once

#include <EASTL/span.h>

#include "KryneEngine/Modules/ShaderReflection/Input/DescriptorInput.hpp"

namespace KryneEngine::Modules::ShaderReflection
{
    struct DescriptorSetInput
    {
        eastl::string_view m_name;
        eastl::span<const DescriptorInput> m_descriptors;
    };
}