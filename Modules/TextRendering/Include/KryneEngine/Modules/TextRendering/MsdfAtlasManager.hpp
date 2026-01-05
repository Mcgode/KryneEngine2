/**
 * @file
 * @author Max Godefroy
 * @date 04/01/2026.
 */

#pragma once

#include "EASTL/vector_map.h"


#include <EASTL/hash_map.h>
#include <KryneEngine/Core/Graphics/GraphicsContext.hpp>
#include <KryneEngine/Core/Graphics/Handles.hpp>
#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>
#include <KryneEngine/Core/Memory/DynamicArray.hpp>
#include <KryneEngine/Core/Threads/SpinLock.hpp>
#include <moodycamel/concurrentqueue.h>

namespace KryneEngine::Modules::TextRendering
{
    class Font;

    class MsdfAtlasManager
    {
    public:
        MsdfAtlasManager(
            AllocatorInstance _allocator,
            GraphicsContext& _graphicsContext,
            u32 _atlasSize,
            u32 _glyphBaseSize);

        ~MsdfAtlasManager();

        struct GlyphRegion
        {
            u16 m_x = 0;
            u16 m_y = 0;
            u16 m_size = 0;
            u16 m_pxRange = 0;
        };

        GlyphRegion GetGlyphRegion(Font* _font, u32 _unicodeCodepoint, u8 _sizeLShift = 1);

        void FlushLoads(GraphicsContext& _graphicsContext, CommandListHandle _transfer);

    private:
        struct StagingBuffer
        {
            BufferHandle m_buffer {};
            u32 m_size = 0;
        };

        struct GlyphKey
        {
            Font* m_font;
            u32 m_unicodeCodepoint;

            inline bool operator==(const GlyphKey& _other) const noexcept
            {
                return m_font == _other.m_font && m_unicodeCodepoint == _other.m_unicodeCodepoint;
            }

            inline bool operator<(const GlyphKey& _other) const noexcept
            {
                return m_font < _other.m_font || (m_font == _other.m_font && m_unicodeCodepoint < _other.m_unicodeCodepoint);
            }

            struct Hasher
            {
                size_t operator()(const GlyphKey& _key) const noexcept;
            };
        };

        struct GlyphSlot
        {
            u32 m_index: 24;
            u32 m_sizeLShift: 8;
        };

        struct GlyphLoadRequest
        {
            GlyphKey m_key;
            float* m_buffer;
        };

        AllocatorInstance m_allocator;
        DynamicArray<StagingBuffer> m_stagingBuffers;
        TextureHandle m_atlasTexture {};
        SubResourceIndexing m_atlasTextureSubresourceIndex {};
        TextureMemoryFootprint m_atlasFootprint {};
        DynamicArray<TextureMemoryFootprint> m_slotFootprints;
        u32 m_atlasSize;
        u32 m_glyphBaseSize;
        DynamicArray<u64> m_slotsBitMap;
        SpinLock m_lock {};
        eastl::vector_map<GlyphKey, GlyphSlot> m_glyphSlotMap;
        moodycamel::ConcurrentQueue<GlyphLoadRequest> m_loadQueue;

        uint2 FindFreeSlot(u8 _slotSize) const;
    };
}
