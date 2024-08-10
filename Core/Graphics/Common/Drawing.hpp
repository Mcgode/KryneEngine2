/**
 * @file
 * @author Max Godefroy
 * @date 10/08/2024.
 */

#pragma once

#include <Common/Types.hpp>

namespace KryneEngine
{
    struct Viewport
    {
        s32 m_topLeftX = 0;
        s32 m_topLeftY = 0;
        s32 m_width;
        s32 m_height;
        float m_minDepth = 0.f;
        float m_maxDepth = 1.f;
    };
}
