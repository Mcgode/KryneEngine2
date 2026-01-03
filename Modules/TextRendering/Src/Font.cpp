/**
 * @file
 * @author Max Godefroy
 * @date 03/01/2026.
 */

#include "KryneEngine/Modules/TextRendering/Font.hpp"

namespace KryneEngine::Modules::TextRendering
{
    Font::Font(AllocatorInstance _allocator)
        : m_points(_allocator)
        , m_tags(_allocator)
        , m_glyphs(_allocator)
    {}
} // namespace KryneEngine::Modules::TextRendering