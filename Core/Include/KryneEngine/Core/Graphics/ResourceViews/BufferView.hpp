/**
 * @file
 * @author Max Godefroy
 * @date 30/04/2025.
 */

#pragma once

#include "KryneEngine/Core/Common/BitUtils.hpp"
#include "KryneEngine/Core/Graphics/Handles.hpp"

namespace KryneEngine
{
    enum class BufferViewAccessType: u8
    {
        Read        = 1 << 0,
        Write       = 1 << 1,
        ReadWrite   = Read | Write,
        Constant    = 1 << 2, // For constant buffer access
    };
    KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(BufferViewAccessType)

    struct BufferViewDesc
    {
        BufferHandle m_buffer;
        size_t m_size;
        size_t m_offset;
        u32 m_stride;
        BufferViewAccessType m_accessType;
#if !defined(KE_FINAL)
        eastl::string_view m_debugName;
#endif
    };
}