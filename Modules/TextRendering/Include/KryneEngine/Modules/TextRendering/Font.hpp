/**
 * @file
 * @author Max Godefroy
 * @date 03/01/2026.
 */

#pragma once

#include "KryneEngine/Core/Common/Utils/Macros.hpp"


#include <EASTL/vector_map.h>
#include <KryneEngine/Core/Math/Vector.hpp>
#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>
#include <KryneEngine/Core/Memory/UniquePtr.hpp>
#include <KryneEngine/Core/Threads/SpinLock.hpp>
#include <atomic>

struct FT_FaceRec_;

namespace KryneEngine::Modules::TextRendering
{
    class Font
    {
        friend class FontManager;

    public:
        ~Font();

        float GetAscender(float _fontSize) const;
        float GetDescender(float _fontSize) const;
        float GetLineHeight(float _fontSize) const;

        float GetHorizontalAdvance(u32 _unicodeCodepoint, float _fontSize);

    private:
        explicit Font(AllocatorInstance _allocator);

        enum class OutlineTag: u8
        {
            NewContour,
            Line,
            Conic,
            Cubic
        };

        struct GlyphEntry
        {
            u32 m_glyphIndex;
            // Should be accessed as atomic ref in concurrent contexts. We don't store it as a std::atomic to allow for
            // vector map sorting
            bool m_loaded = false;

            u32 m_baseAdvanceX;

            u32 m_baseBearingX;
            u32 m_baseWidth;

            u32 m_baseBearingY;
            u32 m_baseHeight;

            u32 m_outlineStartPoint;
            u32 m_outlineFirstTag;
            u32 m_outlineTagCount;
        };

        FT_FaceRec_* m_face = nullptr;
        std::byte* m_fileBuffer = nullptr;
        AllocatorInstance m_fileBufferAllocator {};
        eastl::vector<uint2> m_points;
        eastl::vector<OutlineTag> m_tags;
        eastl::vector_map<u32, GlyphEntry> m_glyphs;
        SpinLock m_loadLock {};
        SpinLock m_outlinesLock {};

        void LoadGlyph(size_t _vectorMapIndex);
        void LoadGlyphSafe(size_t _vectorMapIndex);
    };
}
