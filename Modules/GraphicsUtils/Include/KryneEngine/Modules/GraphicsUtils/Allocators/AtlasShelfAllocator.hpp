/**
 * @file
 * @author Max Godefroy
 * @date 08/01/2026.
 */

#pragma once

#include <EASTL/vector_map.h>

#include <KryneEngine/Core/Math/Vector.hpp>
#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>
#include <KryneEngine/Core/Memory/Containers/VectorDeLinkedList.hpp>

namespace KryneEngine::Modules::GraphicsUtils
{
    class AtlasShelfAllocator
    {
        friend class AtlasShelfAllocatorExplorator; // For unit tests

    public:
        struct Configuration
        {
            uint2 m_atlasSize = { 1024, 1024 };
            u32 m_shelfWidth = 512;
            u32 m_minHeight = 16;
            u32 m_slWidth = 2;
        };

        AtlasShelfAllocator(AllocatorInstance _cpuAllocator, const Configuration& _config);

        u32 Allocate(uint2 _slotSize);
        void Free(u32 _slot);

    private:
        struct FreeShelfEntry
        {
            u32 m_start = 0;
            u32 m_size = 0;
        };

        struct ShelfEntry
        {
            u32 m_start = 0;
            u32 m_size = 0;
            u32 m_firstFree = 0;
            u32 m_next = 0;
            u32 m_previous = 0;
        };

        struct FreeSlotEntry
        {
            u32 m_start = 0;
            u32 m_width = 0;
            u32 m_next = 0;
            u32 m_previous = 0;
        };

        struct SlotEntry
        {
            u32 m_shelf = ~0u;
            u32 m_start = 0;
            u32 m_width = 0;
        };

        static constexpr u32 kBlockAlignment = 4;
        static constexpr u32 kInvalidSlot = ~0u;

        AllocatorInstance m_cpuAllocator;
        uint2 m_atlasSize;
        u32 m_shelfWidth;
        u32 m_shelfColumns;
        u32 m_minHeight;
        u32 m_slWidth;
        eastl::vector<FreeShelfEntry> m_freeShelves;
        VectorDeLinkedList<ShelfEntry> m_shelves;
        eastl::vector_map<u32, u32> m_shelfCategories;
        VectorDeLinkedList<FreeSlotEntry> m_freeSlots;
        eastl::vector<SlotEntry> m_slots;
        u32 m_nextSlotIndex = kInvalidSlot;

        void FreeShelf(FreeShelfEntry _freedShelf);

        u32 FindSlot(u32 _width, u32 _shelfIndex, bool _allocateShelfIfNeeded);
        u32 TryAllocateShelf(u32 _height);
        u32 AllocateSlot();
    };

}
