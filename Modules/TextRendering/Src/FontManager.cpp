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

        // Select best charmap
        {
            s32 bestCharMap = -1;
            s32 bestPriority = eastl::numeric_limits<s32>::max();
            for (s32 charMapIndex = 0; charMapIndex < face->num_charmaps; ++charMapIndex)
            {
                FT_CharMap const charMap = face->charmaps[charMapIndex];
                if (charMap->encoding != FT_ENCODING_UNICODE)
                {
                    continue;
                }

                s32 priority;
                if (charMap->platform_id == 3 && charMap->encoding_id == 10) // Microsoft UTF-32
                    priority = 0;
                else if (charMap->platform_id == 3) // Apple
                    priority = 10;
                else if (charMap->platform_id == 1 && charMap->encoding_id == 1) // Microsoft UTF-16
                    priority = 20;
                else
                    priority = 50 + charMapIndex; // Tie-breaker

                if (priority < bestPriority)
                {
                    bestPriority = priority;
                    bestCharMap = charMapIndex;
                }
            }

            if (!KE_VERIFY_MSG(bestCharMap >= 0, "No available unicode char map")) [[unlikely]]
            {
                KE_VERIFY(FT_Done_Face(face) == FT_Err_Ok);
                return nullptr;
            }
            const FT_Error error = FT_Set_Charmap(face, face->charmaps[bestCharMap]);
            if (!KE_VERIFY_MSG(error == FT_Err_Ok, FT_Error_String(error))) [[unlikely]]
            {
                KE_VERIFY(FT_Done_Face(face) == FT_Err_Ok);
                return nullptr;
            }
        }

        auto* newFont = new (m_allocator.Allocate<Font>()) Font(m_allocator);
        newFont->m_face = face;
        newFont->m_fileBuffer = reinterpret_cast<std::byte*>(buffer);
        newFont->m_fileBufferAllocator = m_allocator;

        // Parse all glyphs
        u32 glyphIndex;
        u32 unicodeCodepoint = FT_Get_First_Char(face, &glyphIndex);
        while (glyphIndex != 0)
        {
            const FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_BITMAP);
            if (!KE_VERIFY_MSG(error == FT_Err_Ok, FT_Error_String(error))) [[unlikely]]
            {
                m_allocator.Delete(newFont);
                KE_VERIFY(FT_Done_Face(face) == FT_Err_Ok);
                return nullptr;
            }

            auto& pair = newFont->m_glyphs.emplace_back_unsorted(unicodeCodepoint, Font::GlyphEntry {});
            pair.second.m_glyphIndex = glyphIndex;

            if (const bool preload = unicodeCodepoint < 128) // Preload all ASCII chars
            {
                newFont->LoadGlyph(newFont->m_glyphs.size() - 1);

                // Can store non-atomically here, since we are in a non-concurrent context.
                pair.second.m_loaded = true;
            }

            unicodeCodepoint = FT_Get_Next_Char(face, unicodeCodepoint, &glyphIndex);
        }

        // Sort vector map
        eastl::sort(
            newFont->m_glyphs.begin(),
            newFont->m_glyphs.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

        newFont->m_fontId = m_fonts.size();
        m_fonts.push_back(newFont);
        return newFont;
    }

    Font* FontManager::GetFont(const u16 _fontId)
    {
        return m_fonts[_fontId];
    }
} // namespace KryneEngine::Modules::TextRendering