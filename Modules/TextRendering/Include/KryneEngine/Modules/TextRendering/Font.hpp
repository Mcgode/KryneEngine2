/**
 * @file
 * @author Max Godefroy
 * @date 03/01/2026.
 */

#pragma once

#include <EASTL/span.h>
#include <EASTL/vector_map.h>
#include <KryneEngine/Core/Math/Vector.hpp>
#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>
#include <KryneEngine/Core/Threads/SpinLock.hpp>

struct FT_FaceRec_;

namespace KryneEngine::Modules::TextRendering
{
    class Font
    {
        friend class FontManager;

    public:
        ~Font();

        struct GlyphLayoutMetrics
        {
            float m_advanceX;
            float m_bearingX;
            float m_width;
            float m_bearingY;
            float m_height;
        };

        float GetAscender(float _fontSize) const;
        float GetDescender(float _fontSize) const;
        float GetLineHeight(float _fontSize) const;

        float GetHorizontalAdvance(u32 _unicodeCodepoint, float _fontSize);
        GlyphLayoutMetrics GetGlyphLayoutMetrics(u32 _unicodeCodepoint, float _fontSize);

        bool GenerateMsdf(u32 _unicodeCodepoint, u16 _glyphSize, u16 _pxRange, eastl::span<float> _output);

        [[nodiscard]] u16 GeId() const { return m_fontId; }

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

        u16 m_fontId = 0;
        FT_FaceRec_* m_face = nullptr;
        std::byte* m_fileBuffer = nullptr;
        AllocatorInstance m_fileBufferAllocator {};
        eastl::vector<int2> m_points;
        eastl::vector<OutlineTag> m_tags;
        eastl::vector_map<u32, GlyphEntry> m_glyphs;
        SpinLock m_loadLock {};
        SpinLock m_outlinesLock {};

        void LoadGlyph(size_t _vectorMapIndex);
        void LoadGlyphSafe(size_t _vectorMapIndex);
    };
}
