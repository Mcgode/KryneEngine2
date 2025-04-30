/**
 * @file
 * @author Max Godefroy
 * @date 14/04/2025.
 */

#pragma once

#include "KryneEngine/Core/Graphics/ShaderPipeline.hpp"
#include <EASTL/optional.h>

#include "KryneEngine/Modules/ShaderReflection/Input/PushConstantInput.hpp"
#include "KryneEngine/Modules/ShaderReflection/Input/DescriptorSetInput.hpp"

namespace KryneEngine::Modules::ShaderReflection
{
    struct EntryPointInput
    {
        eastl::string_view m_name;
        ShaderStage::Stage m_stage;
        eastl::optional<PushConstantInput> m_pushConstants;
        eastl::span<const DescriptorSetInput> m_descriptorSets;
    };
}