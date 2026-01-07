/**
 * @file
 * @author Max Godefroy
 * @date 03/01/2026.
 */

#include "KryneEngine/Modules/TextRendering/Font.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include <msdfgen.h>

namespace KryneEngine::Modules::TextRendering
{
    Font::~Font()
    {
        FT_Done_Face(m_face);
        m_fileBufferAllocator.deallocate(m_fileBuffer);
    }

    float Font::GetAscender(float _fontSize) const
    {
        return _fontSize * static_cast<float>(m_face->ascender) / static_cast<float>(m_face->units_per_EM);
    }

    float Font::GetDescender(float _fontSize) const
    {
        return _fontSize * static_cast<float>(m_face->descender) / static_cast<float>(m_face->units_per_EM);
    }

    float Font::GetLineHeight(float _fontSize) const
    {
        return _fontSize * static_cast<float>(m_face->height) / static_cast<float>(m_face->units_per_EM);
    }

    float Font::GetHorizontalAdvance(const u32 _unicodeCodepoint, const float _fontSize)
    {
        auto it = m_glyphs.find(_unicodeCodepoint);
        if (it != m_glyphs.end() || m_glyphs.begin()->first == 0)
        {
            if (it == m_glyphs.end())
                it = m_glyphs.begin();
            GlyphEntry& entry = it->second;

            // Atomic relaxed load to check if loaded. If not, cache it.
            if (std::atomic_ref(entry.m_loaded).load(std::memory_order_relaxed) == false) [[unlikely]]
                LoadGlyphSafe(eastl::distance(m_glyphs.begin(), it));

            return _fontSize * static_cast<float>(entry.m_baseAdvanceX) / static_cast<float>(m_face->units_per_EM);
        }
        return 0;
    }

    Font::GlyphLayoutMetrics Font::GetGlyphLayoutMetrics(u32 _unicodeCodepoint, float _fontSize)
    {
        auto it = m_glyphs.find(_unicodeCodepoint);
        if (it != m_glyphs.end() || m_glyphs.begin()->first == 0)
        {
            if (it == m_glyphs.end())
                it = m_glyphs.begin();
            GlyphEntry& entry = it->second;

            // Atomic relaxed load to check if loaded. If not, cache it.
            if (std::atomic_ref(entry.m_loaded).load(std::memory_order_relaxed) == false) [[unlikely]]
                LoadGlyphSafe(eastl::distance(m_glyphs.begin(), it));

            const float emScale = 1.f / static_cast<float>(m_face->units_per_EM);
            return {
                _fontSize * emScale * static_cast<float>(entry.m_baseAdvanceX),
                _fontSize * emScale * static_cast<float>(entry.m_baseBearingX),
                _fontSize * emScale * static_cast<float>(entry.m_baseWidth),
                _fontSize * emScale * static_cast<float>(entry.m_baseBearingY),
                _fontSize * emScale * static_cast<float>(entry.m_baseHeight)
            };
        }
        return { 0, 0, 0, 0, 0 };
    }

    bool Font::GenerateMsdf(
        const u32 _unicodeCodepoint,
        const u16 _glyphSize,
        const u16 _pxRange,
        const eastl::span<float> _output)
    {
        KE_ASSERT(_output.size() >= _glyphSize * _glyphSize * 3);

        KE_ZoneScopedF("Generate MSDF for U+%x", _unicodeCodepoint);

        const auto it = m_glyphs.find(_unicodeCodepoint);
        if (it == m_glyphs.end())
            return false;

        GlyphEntry& entry = it->second;
        if (std::atomic_ref(entry.m_loaded).load(std::memory_order_relaxed) == false) [[unlikely]]
            LoadGlyphSafe(eastl::distance(m_glyphs.begin(), it));

        msdfgen::Vector2 scale = 1;
        msdfgen::Vector2 translate = 0;
        {
            const auto glyphWidth = static_cast<double>(entry.m_baseWidth);
            const auto glyphXMin = static_cast<double>(entry.m_baseBearingX);
            const auto glyphHeight = static_cast<double>(entry.m_baseHeight);
            const auto glyphYMin = static_cast<double>(entry.m_baseBearingY) - glyphHeight;

            const auto dims = static_cast<double>(_glyphSize - _pxRange);

            if (glyphHeight > glyphWidth)
            {
                translate.set(0.5 * (glyphHeight - glyphWidth) - glyphXMin, -glyphYMin);
                scale = dims / glyphHeight;
            }
            else
            {
                translate.set(-glyphXMin, 0.5 * (glyphWidth - glyphHeight) - glyphYMin);
                scale = dims / glyphWidth;
            }
            translate += (_pxRange * 0.5) / scale;
        }

        msdfgen::Shape shape;
        {
            KE_ZoneScoped("Retrieve shape");

            const auto lock = m_outlinesLock.AutoLock();

            const int2* pPoints = m_points.data() + entry.m_outlineStartPoint;
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

        KE_ASSERT(shape.validate());

        msdfgen::edgeColoringByDistance(shape, 3);

        const msdfgen::SDFTransformation transformation {
            msdfgen::Projection(scale, translate),
            msdfgen::Range(_pxRange / scale.x)
        };
        const msdfgen::BitmapSection<float, 3> bitmapSection { _output.data(), _glyphSize, _glyphSize, msdfgen::Y_DOWNWARD };
        msdfgen::MSDFGeneratorConfig generatorConfig {
            true,
            msdfgen::ErrorCorrectionConfig { msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY }
        };
        msdfgen::generateMSDF(
            bitmapSection,
            shape,
            transformation,
            generatorConfig);

        return true;
    }

    Font::Font(AllocatorInstance _allocator)
        : m_points(_allocator)
        , m_tags(_allocator)
        , m_glyphs(_allocator)
    {
    }

    void Font::LoadGlyph(size_t _vectorMapIndex)
    {
        GlyphEntry& glyphEntry = (m_glyphs.begin() + _vectorMapIndex)->second;

        if (m_face->glyph == nullptr || m_face->glyph->glyph_index != glyphEntry.m_glyphIndex)
        {
            {
                const FT_Error error = FT_Load_Glyph(m_face, glyphEntry.m_glyphIndex, FT_LOAD_NO_BITMAP);
                KE_ASSERT_MSG(error == FT_Err_Ok, FT_Error_String(error));
            }
        }

        const FT_GlyphSlot glyph = m_face->glyph;
        const FT_Outline outline = glyph->outline;

        glyphEntry.m_baseAdvanceX = glyph->metrics.horiAdvance;

        glyphEntry.m_baseBearingX = glyph->metrics.horiBearingX;
        glyphEntry.m_baseWidth = glyph->metrics.width;

        glyphEntry.m_baseBearingY = glyph->metrics.horiBearingY;
        glyphEntry.m_baseHeight = glyph->metrics.height;

        m_outlinesLock.Lock();

        glyphEntry.m_outlineFirstTag = m_tags.size();
        glyphEntry.m_outlineStartPoint = m_points.size();

        // Based on `FT_Outline_Decompose()` implementation
        for (u32 i = 0; i < outline.n_contours; i++)
        {
            u32 start = i > 0 ? outline.contours[i - 1] + 1 : 0;
            u32 last = outline.contours[i];

            u8 tag = FT_CURVE_TAG(outline.tags[start]);

            uint2_simd vStart { outline.points[start].x, outline.points[start].y };
            uint2_simd vLast { outline.points[last].x, outline.points[last].y };
            uint2_simd vControl = vStart;

            FT_Vector* pPoints = outline.points + start;
            u8* pTags = outline.tags + start;
            FT_Vector* end = outline.points + last;

            KE_ASSERT(tag != FT_CURVE_TAG_CUBIC);

            if (tag == FT_CURVE_TAG_CONIC)
            {
                if (FT_CURVE_TAG(outline.tags[last]) == FT_CURVE_TAG_ON)
                {
                    vStart = vLast;
                    end--;
                }
                else
                {
                    vStart = (vStart + vLast) / uint2_simd(2);
                }
                pPoints--;
                pTags--;
            }

            // First point of the contour
            {
                m_tags.push_back(OutlineTag::NewContour);
                m_points.emplace_back(vStart);
            }

            while (pPoints < end)
            {
                pPoints++;
                pTags++;

                tag = FT_CURVE_TAG(*pTags);
                switch (tag)
                {
                case FT_CURVE_TAG_ON:
                    m_tags.push_back(OutlineTag::Line);
                    m_points.emplace_back(pPoints->x, pPoints->y);

                    // Close contour
                    if (pPoints == end)
                    {
                        m_tags.push_back(OutlineTag::Line);
                        m_points.emplace_back(vStart);
                    }
                    break;
                case FT_CURVE_TAG_CONIC:
                {
                    m_tags.push_back(OutlineTag::Conic);

                    vControl = { pPoints->x, pPoints->y };
                    m_points.emplace_back(vControl);

                    if (pPoints < end)
                    {
                        tag = FT_CURVE_TAG(pTags[1]);
                        uint2_simd vec { pPoints[1].x, pPoints[1].y };

                        if (tag == FT_CURVE_TAG_ON)
                        {
                            m_points.emplace_back(vec);

                            // We consumed a point, advance.
                            pPoints++;
                            pTags++;
                        }
                        // We are chaining conic arcs, so we take the median point of the two consecutive control points
                        else if (tag == FT_CURVE_TAG_CONIC)
                        {
                            m_points.emplace_back((vControl + vec) / uint2_simd(2));
                            // The control point hasn't been consumed yet (we created a median point instead),
                            // so we don't need to advance
                        }
                        else
                        {
                            KE_ERROR("Invalid tag");
                        }
                    }
                    // If there is no more point available, it means we have closed the contour loop, and the last
                    // point is the start point
                    else
                    {
                        m_points.emplace_back(vStart);
                    }

                    break;
                }
                default: // case FT_CURVE_TAG_CUBIC:
                {
                    KE_ASSERT_FATAL(pPoints + 1 <= end && FT_CURVE_TAG(pTags[1]) == FT_CURVE_TAG_CUBIC);

                    m_tags.push_back(OutlineTag::Cubic);

                    int2_simd v1 = { pPoints->x, pPoints->y };
                    pPoints++;
                    int2_simd v2 = { pPoints->x, pPoints->y };
                    pPoints++;

                    m_points.emplace_back(v1);
                    m_points.emplace_back(v2);

                    if (pPoints <= end)
                    {
                        m_points.emplace_back(pPoints->x, pPoints->y);
                    }
                    // Close contour
                    else
                    {
                        m_points.emplace_back(vStart);
                    }

                    break;
                }
                }
            }

            if (vStart != vLast)
            {
                m_tags.push_back(OutlineTag::Line);
                m_points.emplace_back(vStart);
            }
        }

        glyphEntry.m_outlineTagCount = m_tags.size() - glyphEntry.m_outlineFirstTag;

        m_outlinesLock.Unlock();
    }

    void Font::LoadGlyphSafe(size_t _vectorMapIndex)
    {
        const auto lock = m_loadLock.AutoLock();

        // Check that load hasn't been performed while waiting for spinlock.
        GlyphEntry& glyphEntry = (m_glyphs.begin() + _vectorMapIndex)->second;
        if (std::atomic_ref(glyphEntry.m_loaded).load(std::memory_order_acquire))
        {
            return;
        }

        LoadGlyph(_vectorMapIndex);

        // Load was performed, update status.
        std::atomic_ref(glyphEntry.m_loaded).store(true, std::memory_order_release);
    }
} // namespace KryneEngine::Modules::TextRendering