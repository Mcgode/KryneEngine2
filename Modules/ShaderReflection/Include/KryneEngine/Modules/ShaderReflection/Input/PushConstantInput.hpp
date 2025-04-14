/**
 * @file
 * @author Max Godefroy
 * @date 14/04/2025.
 */

#pragma once

#include <KryneEngine/Core/Common/Types.hpp>
#include <EASTL/string_view.h>

namespace KryneEngine::Modules::ShaderReflection
{
    struct PushConstantInput
    {
        eastl::string_view m_name;
        u64 m_signatureHash;
        u32 m_size;
    };
}