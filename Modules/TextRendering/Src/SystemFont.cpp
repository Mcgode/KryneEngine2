/**
 * @file
 * @author Max Godefroy
 * @date 13/01/2026.
 */

#include "KryneEngine/Modules/TextRendering/SystemFont.hpp"

#include <EASTL/sort.h>
#include <KryneEngine/Core/Memory/DynamicArray.hpp>
#include <KryneEngine/Core/Platform/Platform.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include <msdfgen.h>

namespace KryneEngine::Modules::TextRendering
{
    void FixShapeWinding(msdfgen::Shape& _shape, AllocatorInstance _allocator)
    {
        struct Intersection
        {
            float m_x;
            s8 m_winding;
        };

        eastl::vector<Intersection> intersections(_allocator);
        DynamicArray<s8> orientations(_allocator, _shape.contours.size(), 0);

        for (u32 i = 0; i < _shape.contours.size(); ++i)
        {
            const msdfgen::Contour& contour = _shape.contours[i];

            if (orientations[i] != 0 || contour.edges.empty())
                continue;

            // an irrational number to minimize chance of intersecting a corner or other point of interest
            const double ratio = .5*(sqrt(5)-1);

            double y0 = contour.edges.front()->point(0).y;
            double y1 = y0;

            for (auto it = contour.edges.begin(); it != contour.edges.end() && y0 == y1; ++it)
                y1 = (*it)->point(1).y;

            for (auto it = contour.edges.begin(); it != contour.edges.end() && y0 == y1; ++it)
                y1 = (*it)->point(ratio).y;

            const double y = y0 * ratio + y1 * (1 - ratio);

            double x[3];
            s32 dy[3];
            for (u32 j = 0; j < _shape.contours.size(); ++j)
            {
                for (const auto& edge: _shape.contours[j].edges)
                {
                    const s32 n = edge->scanlineIntersections(x, dy, y);
                    for (s32 k = 0; k < n; ++k)
                        intersections.emplace_back(Intersection {
                            .m_x = static_cast<float>(x[k]),
                            .m_winding = static_cast<s8>(dy[k]),
                        });
                }
            }

            if (intersections.empty())
                continue;

            const Intersection* outerIntersection = eastl::min_element(
                intersections.begin(), intersections.end(), [](const Intersection& a, const Intersection& b) {
                    return a.m_x < b.m_x;
                });

            constexpr s8 expectedWinding = 1;
            const bool isExpectedWinding = outerIntersection->m_winding == expectedWinding;
            if (!isExpectedWinding)
            {
                for (auto& contourToReverse: _shape.contours)
                    contourToReverse.reverse();
            }
            return;
        }
    }

    float SystemFont::GetHorizontalAdvance(const u32 _unicodeCodePoint, const float _fontSize)
    {
        const auto lock = m_lock.AutoLock();
        const auto it = m_glyphs.find(_unicodeCodePoint);
        if (it != m_glyphs.end())
            return static_cast<float>(it->second.m_advanceX) * _fontSize / static_cast<float>(it->second.m_unitsPerEm);

        const GlyphEntry& entry = RetrieveGlyph(_unicodeCodePoint);
        return static_cast<float>(entry.m_advanceX) * _fontSize / static_cast<float>(entry.m_unitsPerEm);
    }

    GlyphLayoutMetrics SystemFont::GetGlyphLayoutMetrics(u32 _unicodeCodePoint, float _fontSize)
    {
        const auto lock = m_lock.AutoLock();
        const auto it = m_glyphs.find(_unicodeCodePoint);
        if (it != m_glyphs.end())
        {
            const GlyphEntry& entry = it->second;
            const float scale = _fontSize / static_cast<float>(entry.m_unitsPerEm);
            return {
                .m_advanceX = static_cast<float>(entry.m_advanceX) * scale,
                .m_bearingX = static_cast<float>(entry.m_bearingX) * scale,
                .m_width = static_cast<float>(entry.m_width) * scale,
                .m_bearingY = static_cast<float>(entry.m_bearingY) * scale,
                .m_height = static_cast<float>(entry.m_height) * scale
            };
        }

        const GlyphEntry& entry = RetrieveGlyph(_unicodeCodePoint);
        const float scale = _fontSize / static_cast<float>(entry.m_unitsPerEm);
        return {
            .m_advanceX = static_cast<float>(entry.m_advanceX) * scale,
            .m_bearingX = static_cast<float>(entry.m_bearingX) * scale,
            .m_width = static_cast<float>(entry.m_width) * scale,
            .m_bearingY = static_cast<float>(entry.m_bearingY) * scale,
            .m_height = static_cast<float>(entry.m_height) * scale
        };
    }

    float* SystemFont::GenerateMsdf(u32 _unicodeCodepoint, float _fontSize, u16 _pxRange, AllocatorInstance _allocator)
    {
        const auto lock = m_lock.AutoLock();
        const auto it = m_glyphs.find(_unicodeCodepoint);
        const GlyphEntry& entry = it != m_glyphs.end() ? it->second : RetrieveGlyph(_unicodeCodepoint);

        if (entry.m_outlineTagCount == 0)
            return nullptr;

        KE_ZoneScopedF("Generate MSDF for U+%x", _unicodeCodepoint);

        const double fontScale = _fontSize / static_cast<double>(entry.m_unitsPerEm);

        msdfgen::Vector2 scale = 1;
        msdfgen::Vector2 translate = 0;

        const auto pxRange = static_cast<double>(_pxRange);
        const auto glyphWidth = fontScale * static_cast<double>(entry.m_width);
        const auto glyphHeight = fontScale * static_cast<double>(entry.m_height);
        const auto glyphYBearing = fontScale * static_cast<double>(entry.m_bearingY);

        const double baseLineYOffset = std::ceil(glyphHeight - glyphYBearing);

        const uint2 finalGlyphDims {
            std::ceil(glyphWidth) + _pxRange,
            baseLineYOffset + std::ceil(glyphYBearing) + _pxRange
        };

        scale = fontScale;
        translate.set(
            -static_cast<double>(entry.m_bearingX),
            baseLineYOffset / fontScale);
        translate += (pxRange * 0.5) / scale;

        msdfgen::Shape shape;
        {
            KE_ZoneScoped("Retrieve shape");

            const int2* pPoints = m_glyphPositions.data() + entry.m_outlineStartPoint;
            const OutlineTag* pTags = m_tags.data() + entry.m_outlineFirstTag;
            const OutlineTag* pTagsEnd = pTags + entry.m_outlineTagCount;

            msdfgen::Vector2 currentPoint;

            for (; pTags < pTagsEnd; pTags++)
            {
                switch (*pTags)
                {
                case OutlineTag::NewContour:
                    shape.addContour();
                    currentPoint.set(pPoints->x, pPoints->y);
                    pPoints++;
                    break;
                case OutlineTag::Line:
                {
                    const msdfgen::Vector2 nextPoint(pPoints->x, pPoints->y);
                    shape.contours.back().addEdge(msdfgen::EdgeHolder(currentPoint, nextPoint));
                    currentPoint = nextPoint;
                    pPoints++;
                    break;
                }
                case OutlineTag::Conic:
                {
                    const msdfgen::Vector2 controlPoint(pPoints[0].x, pPoints[0].y);
                    const msdfgen::Vector2 nextPoint(pPoints[1].x, pPoints[1].y);
                    shape.contours.back().addEdge(msdfgen::EdgeHolder(currentPoint, controlPoint, nextPoint));
                    currentPoint = nextPoint;
                    pPoints += 2;
                    break;
                }
                case OutlineTag::Cubic:
                {
                    const msdfgen::Vector2 controlPoint0(pPoints[0].x, pPoints[0].y);
                    const msdfgen::Vector2 controlPoint1(pPoints[1].x, pPoints[1].y);
                    const msdfgen::Vector2 nextPoint(pPoints[2].x, pPoints[2].y);
                    shape.contours.back().addEdge(msdfgen::EdgeHolder(currentPoint, controlPoint0, controlPoint1, nextPoint));
                    currentPoint = nextPoint;
                    pPoints += 3;
                    break;
                }
                }
            }
        }

        FixShapeWinding(shape, m_tags.get_allocator());

        KE_ASSERT(shape.validate());

        msdfgen::edgeColoringByDistance(shape, 3);

        const msdfgen::SDFTransformation transformation {
            msdfgen::Projection(scale, translate),
            msdfgen::Range(_pxRange / scale.x)
        };
        auto* pixels = _allocator.Allocate<float>(3 * finalGlyphDims.x * finalGlyphDims.y);
        const msdfgen::BitmapSection<float, 3> bitmapSection {
            pixels,
            static_cast<s32>(finalGlyphDims.x),
            static_cast<s32>(finalGlyphDims.y),
            msdfgen::Y_DOWNWARD
        };
        msdfgen::MSDFGeneratorConfig generatorConfig {
            true,
            msdfgen::ErrorCorrectionConfig { msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY }
        };
        msdfgen::generateMSDF(
            bitmapSection,
            shape,
            transformation,
            generatorConfig);

        return pixels;
    }

    SystemFont::SystemFont(const AllocatorInstance _allocator)
        : m_glyphs(_allocator)
        , m_tags(_allocator)
        , m_glyphPositions(_allocator)
    {
    }

    const SystemFont::GlyphEntry& SystemFont::RetrieveGlyph(u32 _unicodeCodePoint)
    {
        GlyphEntry& entry = m_glyphs[_unicodeCodePoint];

        struct GlyphEntryRetriever
        {
            SystemFont* m_font;
            GlyphEntry& m_entry;
            int2 m_contourStart {};

            GlyphEntryRetriever(SystemFont* _font, GlyphEntry& _entry, u32 _codePoint)
                : m_font(_font)
                , m_entry(_entry)
            {
                m_entry.m_outlineFirstTag = m_font->m_tags.size();
                m_entry.m_outlineStartPoint = m_font->m_glyphPositions.size();
                Platform::RetrieveSystemDefaultGlyph(
                    _codePoint,
                    this,
                    ReceiveMetrics,
                    NewContour,
                    NewEdge,
                    NewConic,
                    NewCubic,
                    EndContour);

                m_entry.m_outlineTagCount = m_font->m_tags.size() - m_entry.m_outlineFirstTag;
            }

            static void ReceiveMetrics(
                const Platform::FontMetrics& _fontMetrics,
                const Platform::GlyphMetrics& _glyphMetrics,
                void* _userData)
            {
                const auto* self = static_cast<GlyphEntryRetriever*>(_userData);

                self->m_entry.m_fontAscender = static_cast<u32>(_fontMetrics.m_ascender);
                self->m_entry.m_fontDescender = static_cast<u32>(_fontMetrics.m_descender);
                self->m_entry.m_fontLineHeight = static_cast<u32>(_fontMetrics.m_lineHeight);
                self->m_entry.m_unitsPerEm = static_cast<u32>(_fontMetrics.m_unitPerEm);
                self->m_entry.m_advanceX = static_cast<s32>(_glyphMetrics.m_advance);

                self->m_entry.m_bearingX = static_cast<s32>(_glyphMetrics.m_bounds.x);
                self->m_entry.m_bearingY = static_cast<s32>(_glyphMetrics.m_bounds.y + _glyphMetrics.m_bounds.w);
                self->m_entry.m_width = static_cast<u32>(_glyphMetrics.m_bounds.z);
                self->m_entry.m_height = static_cast<u32>(_glyphMetrics.m_bounds.w);
            }

            static void NewContour(const double2& _point, void* _userData)
            {
                auto* self = static_cast<GlyphEntryRetriever*>(_userData);

                self->m_font->m_tags.push_back(OutlineTag::NewContour);
                self->m_font->m_glyphPositions.emplace_back(static_cast<int2>(_point));

                self->m_contourStart = self->m_font->m_glyphPositions.back();
            }

            static void NewEdge(const double2& _point, void* _userData)
            {
                const auto* self = static_cast<GlyphEntryRetriever*>(_userData);

                self->m_font->m_tags.push_back(OutlineTag::Line);
                self->m_font->m_glyphPositions.emplace_back(static_cast<int2>(_point));
            }

            static void NewConic(const double2& _control, const double2& _point, void* _userData)
            {
                const auto* self = static_cast<GlyphEntryRetriever*>(_userData);

                self->m_font->m_tags.push_back(OutlineTag::Conic);
                self->m_font->m_glyphPositions.emplace_back(static_cast<int2>(_control));
                self->m_font->m_glyphPositions.emplace_back(static_cast<int2>(_point));
            }

            static void NewCubic(const double2& _control1, const double2& _control2, const double2& _point, void* _userData)
            {
                const auto* self = static_cast<GlyphEntryRetriever*>(_userData);

                self->m_font->m_tags.push_back(OutlineTag::Cubic);
                self->m_font->m_glyphPositions.emplace_back(static_cast<int2>(_control1));
                self->m_font->m_glyphPositions.emplace_back(static_cast<int2>(_control2));
                self->m_font->m_glyphPositions.emplace_back(static_cast<int2>(_point));
            }

            static void EndContour(void* _userData)
            {
                const auto* self = static_cast<GlyphEntryRetriever*>(_userData);
                const int2& lastPoint = self->m_font->m_glyphPositions.back();
                if (lastPoint != self->m_contourStart)
                {
                    self->m_font->m_tags.push_back(OutlineTag::Line);
                    self->m_font->m_glyphPositions.emplace_back(self->m_contourStart);
                }
            }
        };

        GlyphEntryRetriever { this, entry, _unicodeCodePoint };

        return entry;
    }
}