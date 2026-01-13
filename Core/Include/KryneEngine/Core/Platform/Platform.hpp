/**
 * @file
 * @author Max Godefroy
 * @date 12/01/2026.
 */

#pragma once

#include <EASTL/span.h>

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Math/Vector.hpp"
#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"

namespace KryneEngine::Platform
{

    /**
     * @defgroup Font methods
     * @{
     */

    struct FontMetrics
    {
        double m_ascender;
        double m_descender;
        double m_lineHeight;
    };

    struct GlyphMetrics
    {
        double4 m_bounds {};
        double m_advance {};
    };

    using FontGlyphMetricsFunction = void (*)(const FontMetrics&, const GlyphMetrics&, void*);
    using FontNewContourFunction = void (*)(const double2&, void*);
    using FontNewEdgeFunction = void (*)(const double2&, void*);
    using FontNewConicFunction = void (*)(const double2&, const double2&, void*);
    using FontNewCubicFunction = void (*)(const double2&, const double2&, const double2&, void*);
    using FontEndContourFunction = void (*)(void*);

    /**
     * @brief A function that retrieves glyph data from the system default font.
     *
     * @returns `true` if the glyph was successfully retrieved, `false` otherwise.
     */
    bool RetrieveSystemDefaultGlyph(
        u32 _unicodeCodePoint,
        void* _userData,
        FontGlyphMetricsFunction _fontMetrics,
        FontNewContourFunction _newContour,
        FontNewEdgeFunction _newEdge,
        FontNewConicFunction _newConic,
        FontNewCubicFunction _newCubic,
        FontEndContourFunction _endContour,
        bool _verticalLayout = false);

    /**
     * @}
     */
}