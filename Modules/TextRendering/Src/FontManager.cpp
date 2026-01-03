/**
 * @file
 * @author Max Godefroy
 * @date 03/01/2026.
 */

#include "KryneEngine/Modules/TextRendering/FontManager.hpp"

#include <EASTL/sort.h>
#include <fstream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "KryneEngine/Core/Profiling/TracyHeader.hpp"

#include <KryneEngine/Core/Common/Assert.hpp>

#include "KryneEngine/Modules/TextRendering/Font.hpp"

namespace KryneEngine::Modules::TextRendering
{


    FontManager::FontManager(AllocatorInstance _allocator)
        : m_allocator(_allocator)
        , m_fonts(_allocator)
    {
        const FT_Error error = FT_Init_FreeType(&m_ftLibrary);
        KE_ASSERT_MSG(error == FT_Err_Ok, FT_Error_String(error));
    }

    FontManager::~FontManager()
    {
        for (Font* font : m_fonts)
        {
            font->~Font();
            m_allocator.deallocate(font);
        }
        m_fonts.clear();

        const FT_Error error = FT_Done_FreeType(m_ftLibrary);
        KE_ASSERT_MSG(error == FT_Err_Ok, FT_Error_String(error));
    }

    Font* FontManager::LoadFont(eastl::string_view _path)
    {
        KE_ZoneScopedF("Loading font '%s'", _path.data());

        FT_Face face;
        char* buffer;
        {
            std::ifstream file(_path.data(), std::ios::binary);
            VERIFY_OR_RETURN(file, {});

            file.seekg(0, std::ios::end);
            const std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            buffer = m_allocator.Allocate<char>(size);
            KE_VERIFY(file.read(buffer, size));

            file.close();

            const FT_Error error = FT_New_Memory_Face(m_ftLibrary, reinterpret_cast<FT_Byte*>(buffer), size, 0, &face);

            if (!KE_VERIFY_MSG(error == FT_Err_Ok, "Failed to load font '%s': %s", _path.data(), FT_Error_String(error))) [[unlikely]]
                return nullptr;
        }

        if (!KE_VERIFY_MSG(face->face_flags & FT_FACE_FLAG_SCALABLE, "The API only supports scalable/vector fonts"))
        {
            KE_VERIFY(FT_Done_Face(face) == FT_Err_Ok);
            return nullptr;
        }

        {
            const FT_Error error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
            if (!KE_VERIFY_MSG(error == FT_Err_Ok, FT_Error_String(error))) [[unlikely]]
            {
                KE_VERIFY(FT_Done_Face(face) == FT_Err_Ok);
                return nullptr;
            }
        }

        auto* newFont = new (m_allocator.Allocate<Font>()) Font(m_allocator);

        eastl::vector<uint2> points { m_allocator };
        eastl::vector<Font::OutlineTag> tags { m_allocator };

        // Parse all glyphs
        u32 glyphIndex, unicodeCodepoint = FT_Get_First_Char(face, &glyphIndex);
        while (glyphIndex != 0)
        {
            const FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_BITMAP);
            if (!KE_VERIFY_MSG(error == FT_Err_Ok, FT_Error_String(error))) [[unlikely]]
            {
                m_allocator.Delete(newFont);
                KE_VERIFY(FT_Done_Face(face) == FT_Err_Ok);
                return nullptr;
            }

            // Can sort at the end
            auto& pair = newFont->m_glyphs.emplace_back_unsorted(unicodeCodepoint, Font::GlyphEntry {});

            const FT_GlyphSlot glyph = face->glyph;
            const FT_Outline outline = glyph->outline;

            pair.second.m_outlineFirstTag = tags.size();
            pair.second.m_outlineStartPoint = points.size();

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
                    tags.push_back(Font::OutlineTag::NewContour);
                    points.emplace_back(vStart);
                }

                while (pPoints < end)
                {
                    pPoints++;
                    pTags++;

                    tag = FT_CURVE_TAG(*pTags);
                    switch (tag)
                    {
                    case FT_CURVE_TAG_ON:
                        tags.push_back(Font::OutlineTag::Line);
                        points.emplace_back(pPoints->x, pPoints->y);

                        // Close contour
                        if (pPoints == end)
                        {
                            tags.push_back(Font::OutlineTag::Line);
                            points.emplace_back(vStart);
                        }
                        break;
                    case FT_CURVE_TAG_CONIC:
                    {
                        tags.push_back(Font::OutlineTag::Conic);

                        vControl = { pPoints->x, pPoints->y };
                        points.emplace_back(vControl);

                        if (pPoints < end)
                        {
                            tag = FT_CURVE_TAG(pTags[1]);
                            uint2_simd vec { pPoints[1].x, pPoints[1].y };

                            if (tag == FT_CURVE_TAG_ON)
                            {
                                points.emplace_back(vec);

                                // We consumed a point, advance.
                                pPoints++;
                                pTags++;
                            }
                            // We are chaining conic arcs, so we take the median point of the two consecutive control points
                            else if (tag == FT_CURVE_TAG_CONIC)
                            {
                                points.emplace_back((vControl + vec) / uint2_simd(2));
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
                            points.emplace_back(vStart);
                        }

                        break;
                    }
                    default: // case FT_CURVE_TAG_CUBIC:
                    {
                        KE_ASSERT_FATAL(pPoints + 1 <= end && FT_CURVE_TAG(pTags[1]) == FT_CURVE_TAG_CUBIC);

                        tags.push_back(Font::OutlineTag::Cubic);

                        uint2_simd v1 = { pPoints->x, pPoints->y };
                        pPoints++;
                        uint2_simd v2 = { pPoints->x, pPoints->y };
                        pPoints++;

                        points.emplace_back(v1);
                        points.emplace_back(v2);

                        if (pPoints <= end)
                        {
                            points.emplace_back(pPoints->x, pPoints->y);
                        }
                        // Close contour
                        else
                        {
                            points.emplace_back(vStart);
                        }

                        break;
                    }
                    }
                }
            }

            pair.second.m_outlineTagCount = tags.size() - pair.second.m_outlineFirstTag;

            unicodeCodepoint = FT_Get_Next_Char(face, unicodeCodepoint, &glyphIndex);
        }

        // Sort vector map
        eastl::sort(
            newFont->m_glyphs.begin(),
            newFont->m_glyphs.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

        newFont->m_points.Resize(points.size());
        newFont->m_tags.Resize(tags.size());
        memcpy(newFont->m_points.Data(), points.data(), points.size() * sizeof(uint2));
        memcpy(newFont->m_tags.Data(), tags.data(), tags.size() * sizeof(u8));

        {
            const FT_Error error = FT_Done_Face(face);
            KE_VERIFY_MSG(error == FT_Err_Ok, FT_Error_String(error));
        }
        m_allocator.deallocate(buffer);

        m_fonts.push_back(newFont);
        return newFont;
    }
} // namespace KryneEngine::Modules::TextRendering