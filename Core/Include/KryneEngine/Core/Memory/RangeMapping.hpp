/**
 * @file
 * @author Max Godefroy
 * @date 10/03/2023.
 */

#pragma once

#include "KryneEngine/Core/Common/Types.hpp"

namespace KryneEngine
{
    struct MemoryRangeMapping
    {
        u64 m_size = 0;
        u64 m_offset = 0;
        u8* m_buffer = nullptr;
    };
}