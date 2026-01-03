/**
 * @file
 * @author Max Godefroy
 * @date 03/01/2026.
 */

#pragma once

#include <EASTL/string_view.h>
#include <EASTL/vector.h>
#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>

struct FT_LibraryRec_;

namespace KryneEngine::Modules::TextRendering
{
    class Font;

    class FontManager
    {
    public:
        explicit FontManager(AllocatorInstance _allocator);

        ~FontManager();

        Font* LoadFont(eastl::string_view _path);

    private:
        AllocatorInstance m_allocator;
        eastl::vector<Font*> m_fonts;
        FT_LibraryRec_* m_ftLibrary {};
    };
}
