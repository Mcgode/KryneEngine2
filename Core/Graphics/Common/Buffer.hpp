/**
 * @file
 * @author Max Godefroy
 * @date 03/08/2024.
 */

#pragma once

#include <Graphics/Common/Enums.hpp>

namespace KryneEngine
{
    struct BufferDesc
    {
        u64 m_size;

#if !defined(KE_FINAL)
        eastl::string m_debugName;
#endif
    };

    struct BufferCreateDesc
    {
        BufferDesc m_desc;
        MemoryUsage m_usage;
    };
}