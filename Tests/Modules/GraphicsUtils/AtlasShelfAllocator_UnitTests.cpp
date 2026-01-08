/**
 * @file
 * @author Max Godefroy
 * @date 08/01/2026.
 */

#include <EASTL/span.h>
#include <EASTL/vector_set.h>
#include <gtest/gtest.h>
#include <KryneEngine/Core/Math/Color.hpp>
#include <KryneEngine/Modules/GraphicsUtils/Allocators/AtlasShelfAllocator.hpp>

#include "Utils/AssertUtils.hpp"
#include "Utils/SvgDump.hpp"

namespace KryneEngine::Modules::GraphicsUtils
{
    class AtlasShelfAllocatorExplorator
    {
    public:
        explicit AtlasShelfAllocatorExplorator(AtlasShelfAllocator* _allocator) : m_allocator(_allocator) {}

        [[nodiscard]] u32 GetShelfWidth() const { return m_allocator->m_shelfWidth; }
        [[nodiscard]] u32 GetShelfColumnCount() const { return m_allocator->m_shelfColumns; }
        [[nodiscard]] eastl::span<AtlasShelfAllocator::FreeShelfEntry> GetFreeShelves() const { return m_allocator->m_freeShelves; }

        [[nodiscard]] AtlasShelfAllocator::SlotEntry& GetSlot(const u32 _slotIndex) const { return m_allocator->m_slots[_slotIndex]; }
        [[nodiscard]] AtlasShelfAllocator::ShelfEntry& GetShelf(const u32 _shelfIndex) const { return m_allocator->m_shelves[_shelfIndex]; }
        [[nodiscard]] AtlasShelfAllocator::FreeSlotEntry& GetFreeSlot(const u32 _slotIndex) const { return m_allocator->m_freeSlots[_slotIndex]; }

        void DumpGraph(eastl::string_view _filename, eastl::string_view _title = {}) const
        {
            Tests::SvgDump dumpFile(_filename, _title, m_allocator->m_atlasSize);

            constexpr Color kFreeFillColor = Color(0.5f, 0.5f, 0.5f, 1.f);
            constexpr Color kFreeStrokeColor = Color(0.8f, 0.8f, 0.8f, 1.f);

            for (const auto& freeShelf: m_allocator->m_freeShelves)
            {
                dumpFile.AddRect(
                    freeShelf.m_start / m_allocator->m_atlasSize.y * GetShelfWidth(),
                    freeShelf.m_start % m_allocator->m_atlasSize.y,
                    GetShelfWidth(),
                    freeShelf.m_size,
                    1.f,
                    kFreeFillColor,
                    kFreeStrokeColor);
            }

            for (const auto [category, firstShelfIndex]: m_allocator->m_shelfCategories)
            {
                u32 shelfIndex = firstShelfIndex;
                for (
                    AtlasShelfAllocator::ShelfEntry* shelfEntry = &m_allocator->m_shelves[shelfIndex];
                    shelfIndex != ~0u;
                    shelfIndex = shelfEntry->m_next, shelfEntry = &m_allocator->m_shelves[shelfIndex])
                {
                    float4 shelfRect {
                        shelfEntry->m_start / m_allocator->m_atlasSize.y * GetShelfWidth(),
                        shelfEntry->m_start % m_allocator->m_atlasSize.y,
                        GetShelfWidth(),
                        shelfEntry->m_size,
                    };
                    dumpFile.AddRect(
                        shelfRect.x,
                        shelfRect.y,
                        shelfRect.z,
                        shelfRect.w,
                        1.f,
                        Color(0),
                        Color(0.8f, 0.3f, 0.3f, 1.f));

                    shelfRect.z += shelfRect.x;
                    shelfRect.w += shelfRect.y;

                    u32 freeSlotIndex = shelfEntry->m_firstFree;
                    for (AtlasShelfAllocator::FreeSlotEntry* freeSlotEntry = &m_allocator->m_freeSlots[freeSlotIndex];
                        freeSlotIndex != ~0u;
                        freeSlotIndex = freeSlotEntry->m_next, freeSlotEntry = &m_allocator->m_freeSlots[freeSlotIndex])
                    {
                        float4 slotRect {
                            freeSlotEntry->m_start + shelfRect.x,
                            shelfRect.y + 1,
                            freeSlotEntry->m_start + freeSlotEntry->m_width + shelfRect.x,
                            shelfRect.y + shelfRect.w - 1,
                        };
                        slotRect.x = eastl::max(slotRect.x, shelfRect.x + 1);
                        slotRect.z = eastl::min(slotRect.z, shelfRect.z - 1);

                        slotRect.z -= slotRect.x;
                        slotRect.w -= slotRect.y;

                        dumpFile.AddRect(
                            slotRect.x,
                            slotRect.y,
                            slotRect.z,
                            slotRect.w,
                            1.f,
                            kFreeFillColor,
                            kFreeStrokeColor);
                    }
                }
            }

            eastl::vector_set<u32> unusedSlotIndices;
            for (u32 slotIndex = m_allocator->m_nextSlotIndex; slotIndex != ~0u; slotIndex = m_allocator->m_slots[slotIndex].m_shelf)
            {
                unusedSlotIndices.insert(slotIndex);
            }

            for (u32 slotIndex = 0; slotIndex < m_allocator->m_slots.size(); slotIndex++)
            {
                if (unusedSlotIndices.find(slotIndex) != unusedSlotIndices.end())
                    continue;

                const AtlasShelfAllocator::SlotEntry& slot = m_allocator->m_slots[slotIndex];
                const AtlasShelfAllocator::ShelfEntry& shelf = m_allocator->m_shelves[slot.m_shelf];

                float4 shelfRect {
                    shelf.m_start / m_allocator->m_atlasSize.y * GetShelfWidth(),
                    shelf.m_start % m_allocator->m_atlasSize.y,
                    GetShelfWidth(),
                    shelf.m_size,
                };
                shelfRect.z += shelfRect.x;
                shelfRect.w += shelfRect.y;

                float4 slotRect {
                    slot.m_start + shelfRect.x,
                    shelfRect.y + 1,
                    slot.m_start + slot.m_width + shelfRect.x,
                    shelfRect.y + shelfRect.w - 1,
                };
                slotRect.x = eastl::max(slotRect.x, shelfRect.x + 1);
                slotRect.z = eastl::min(slotRect.z, shelfRect.z - 1);

                slotRect.z -= slotRect.x;
                slotRect.w -= slotRect.y;

                dumpFile.AddRect(
                    slotRect.x,
                    slotRect.y,
                    slotRect.z,
                    slotRect.w,
                    1.f,
                    Color(0.4f, 0.4f, 1.f, 1.f),
                    Color(0.2f, 0.2f, 0.5f, 1.f));
            }
        }

        AtlasShelfAllocator* m_allocator;
    };
}

namespace KryneEngine::Modules::GraphicsUtils::Tests
{
    using namespace KryneEngine::Tests;

    static const AtlasShelfAllocator::Configuration commonConfig {
        .m_atlasSize = { 1024, 1024 },
        .m_shelfWidth = 512,
        .m_minHeight = 16,
        .m_slWidth = 2
    };

    TEST(AtlasShelfAllocatorTests, Initialization)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        AllocatorInstance cpuAllocator;
        AtlasShelfAllocator atlasShelfAllocator(cpuAllocator, commonConfig);
        AtlasShelfAllocatorExplorator explorer(&atlasShelfAllocator);

        EXPECT_EQ(explorer.GetShelfWidth(), commonConfig.m_shelfWidth);
        EXPECT_EQ(explorer.GetShelfColumnCount(), 2);

        const auto freeShelves = explorer.GetFreeShelves();
        EXPECT_EQ(freeShelves.size(), 2); // 2 shelves, 1 per column

        u32 offset = 0;
        EXPECT_EQ(freeShelves[0].m_start, offset);
        EXPECT_EQ(freeShelves[0].m_size, commonConfig.m_atlasSize.y);
        offset += commonConfig.m_atlasSize.y;
        EXPECT_EQ(freeShelves[1].m_start, offset);
        EXPECT_EQ(freeShelves[1].m_size, commonConfig.m_atlasSize.y);

        explorer.DumpGraph("AtlasShelfAllocator_Initialization.svg", "AtlasShelfAllocator Initialization");

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(AtlasShelfAllocatorTests, InitializationCustomConfig)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        AllocatorInstance cpuAllocator;
        const AtlasShelfAllocator::Configuration customConfig {
            .m_atlasSize = { 2048, 512 },
            .m_shelfWidth = 256,
            .m_minHeight = 16,
            .m_slWidth = 2
        };
        AtlasShelfAllocator atlasShelfAllocator(cpuAllocator, customConfig);
        AtlasShelfAllocatorExplorator explorer(&atlasShelfAllocator);

        EXPECT_EQ(explorer.GetShelfWidth(), customConfig.m_shelfWidth);
        EXPECT_EQ(explorer.GetShelfColumnCount(), 8);

        const auto freeShelves = explorer.GetFreeShelves();
        EXPECT_EQ(freeShelves.size(), 8); // 8 shelves, 1 per column

        u32 offset = 0;
        for (auto i = 0; i < 8; i++)
        {
            EXPECT_EQ(freeShelves[i].m_start, offset);
            EXPECT_EQ(freeShelves[i].m_size, customConfig.m_atlasSize.y);
            offset += customConfig.m_atlasSize.y;
        }

        explorer.DumpGraph("AtlasShelfAllocator_InitializationCustomConfig.svg", "AtlasShelfAllocator Initialization Custom Config");

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(AtlasShelfAllocatorTests, SingleAllocate)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        AllocatorInstance cpuAllocator;
        AtlasShelfAllocator atlasShelfAllocator(cpuAllocator, commonConfig);
        AtlasShelfAllocatorExplorator explorer(&atlasShelfAllocator);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        const uint2 size { 32, 128 };
        const u32 allocationSlot = atlasShelfAllocator.Allocate(size);

        EXPECT_EQ(allocationSlot, 0);

        const auto& slotEntry = explorer.GetSlot(allocationSlot);
        EXPECT_EQ(slotEntry.m_shelf, 0);
        EXPECT_EQ(slotEntry.m_start, 0);
        EXPECT_EQ(slotEntry.m_width, size.x);

        const auto& shelfEntry = explorer.GetShelf(slotEntry.m_shelf);
        EXPECT_EQ(shelfEntry.m_start, 0);
        EXPECT_EQ(shelfEntry.m_size, size.y);
        EXPECT_EQ(shelfEntry.m_firstFree, 0);
        EXPECT_EQ(shelfEntry.m_next, ~0u);
        EXPECT_EQ(shelfEntry.m_previous, ~0u);

        const auto& freeSlotEntry = explorer.GetFreeSlot(shelfEntry.m_firstFree);
        EXPECT_EQ(freeSlotEntry.m_start, size.x);
        EXPECT_EQ(freeSlotEntry.m_width, explorer.GetShelfWidth() - size.x);
        EXPECT_EQ(freeSlotEntry.m_next, ~0u);
        EXPECT_EQ(freeSlotEntry.m_previous, ~0u);

        const auto freeShelves = explorer.GetFreeShelves();
        EXPECT_EQ(freeShelves.size(), 2); // 2 shelves, 1 per column

        u32 offset = size.y;
        EXPECT_EQ(freeShelves[0].m_start, offset);
        EXPECT_EQ(freeShelves[0].m_size, commonConfig.m_atlasSize.y - size.y);
        offset += commonConfig.m_atlasSize.y - size.y;
        EXPECT_EQ(freeShelves[1].m_start, offset);
        EXPECT_EQ(freeShelves[1].m_size, commonConfig.m_atlasSize.y);

        explorer.DumpGraph("AtlasShelfAllocator_SingleAllocate.svg", "AtlasShelfAllocator Single Allocate");

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(AtlasShelfAllocatorTests, MultiAllocateSameDims)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        AllocatorInstance cpuAllocator;
        AtlasShelfAllocator atlasShelfAllocator(cpuAllocator, commonConfig);
        AtlasShelfAllocatorExplorator explorer(&atlasShelfAllocator);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        const uint2 size { 32, 128 };
        constexpr u32 count = 4;
        const u32 allocationSlots[count] = {
            atlasShelfAllocator.Allocate(size),
            atlasShelfAllocator.Allocate(size),
            atlasShelfAllocator.Allocate(size),
            atlasShelfAllocator.Allocate(size),
        };

        EXPECT_EQ(allocationSlots[0], 0);
        EXPECT_EQ(allocationSlots[1], 1);
        EXPECT_EQ(allocationSlots[2], 2);
        EXPECT_EQ(allocationSlots[3], 3);

        for (u32 i = 0; i < count; i++)
        {
            const auto& slotEntry = explorer.GetSlot(allocationSlots[i]);
            EXPECT_EQ(slotEntry.m_shelf, 0);
            EXPECT_EQ(slotEntry.m_start, i * size.x);
            EXPECT_EQ(slotEntry.m_width, size.x);
        }

        const auto& shelfEntry = explorer.GetShelf(0);
        EXPECT_EQ(shelfEntry.m_start, 0);
        EXPECT_EQ(shelfEntry.m_size, size.y);
        EXPECT_EQ(shelfEntry.m_firstFree, 0);
        EXPECT_EQ(shelfEntry.m_next, ~0u);
        EXPECT_EQ(shelfEntry.m_previous, ~0u);

        const auto& freeSlotEntry = explorer.GetFreeSlot(shelfEntry.m_firstFree);
        EXPECT_EQ(freeSlotEntry.m_start, size.x * count);
        EXPECT_EQ(freeSlotEntry.m_width, explorer.GetShelfWidth() - size.x * count);
        EXPECT_EQ(freeSlotEntry.m_next, ~0u);
        EXPECT_EQ(freeSlotEntry.m_previous, ~0u);

        const auto freeShelves = explorer.GetFreeShelves();
        EXPECT_EQ(freeShelves.size(), 2); // 2 shelves, 1 per column

        u32 offset = size.y * count;
        EXPECT_EQ(freeShelves[0].m_start, offset);
        EXPECT_EQ(freeShelves[0].m_size, commonConfig.m_atlasSize.y - size.y * count);
        offset += commonConfig.m_atlasSize.y - size.y * count;
        EXPECT_EQ(freeShelves[1].m_start, offset);
        EXPECT_EQ(freeShelves[1].m_size, commonConfig.m_atlasSize.y);

        explorer.DumpGraph("AtlasShelfAllocator_MultiAllocateSameDims.svg");

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(AtlasShelfAllocatorTests, ComplexAllocate)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        AllocatorInstance cpuAllocator;
        AtlasShelfAllocator atlasShelfAllocator(cpuAllocator, commonConfig);
        AtlasShelfAllocatorExplorator explorer(&atlasShelfAllocator);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        eastl::vector<u32> allocations;

        for (u32 i = 0; i < 512; i++)
        {
            const u32 height = (12 + 2 * i) % 128;
            const u32 width = (12 + 3 * i) % 60 + 4;

            allocations.push_back(atlasShelfAllocator.Allocate({ width, height }));
        }

        explorer.DumpGraph("AtlasShelfAllocator_ComplexAllocate.svg", "AtlasShelfAllocator Complex Allocate");

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }
}