/**
 * @file
 * @author Max Godefroy
 * @date 03/01/2026.
 */

#pragma once

#include <EASTL/vector_map.h>
#include <KryneEngine/Core/Math/Vector.hpp>
#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>
#include <KryneEngine/Core/Memory/DynamicArray.hpp>

namespace KryneEngine::Modules::TextRendering
{
    class Font
    {
        friend class FontManager;

    public:
        ~Font() = default;

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
            u32 m_outlineStartPoint;
            u32 m_outlineFirstTag;
            u32 m_outlineTagCount;
        };

        DynamicArray<uint2> m_points;
        DynamicArray<OutlineTag> m_tags;
        eastl::vector_map<u32, GlyphEntry> m_glyphs;
    };
}
