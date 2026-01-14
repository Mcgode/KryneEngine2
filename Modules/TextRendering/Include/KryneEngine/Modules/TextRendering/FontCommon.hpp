/**
 * @file
 * @author Max Godefroy
 * @date 13/01/2026.
 */

#pragma once

#include <KryneEngine/Core/Common/Types.hpp>

namespace KryneEngine::Modules::TextRendering
{
    enum class OutlineTag: u8
    {
        NewContour,
        Line,
        Conic,
        Cubic
    };

    struct GlyphLayoutMetrics
    {
        float m_advanceX;
        float m_bearingX;
        float m_width;
        float m_bearingY;
        float m_height;
    };
}