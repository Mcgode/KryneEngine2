/**
 * @file
 * @author Max Godefroy
 * @date 22/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Graphics/Handles.hpp"

namespace KryneEngine
{
    struct BufferCbvDesc
    {
        BufferHandle m_buffer;
        size_t m_size;
        size_t m_offset;
#if !defined(KE_FINAL)
        eastl::string_view m_debugName;
#endif
    };
}