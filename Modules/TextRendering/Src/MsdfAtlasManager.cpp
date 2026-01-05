/**
 * @file
 * @author Max Godefroy
 * @date 04/01/2026.
 */

#include "KryneEngine/Modules/TextRendering/MsdfAtlasManager.hpp"

#include <KryneEngine/Core/Graphics/Buffer.hpp>
#include <KryneEngine/Core/Graphics/MemoryBarriers.hpp>
#include <KryneEngine/Core/Math/Color.hpp>
#include <KryneEngine/Core/Math/Hashing.hpp>

#include "KryneEngine/Modules/TextRendering/Font.hpp"

namespace KryneEngine::Modules::TextRendering
{
    MsdfAtlasManager::MsdfAtlasManager(
        AllocatorInstance _allocator,
        GraphicsContext& _graphicsContext,
        u32 _atlasSize,
        u32 _glyphBaseSize)
            : m_allocator(_allocator)
            , m_stagingBuffers(_allocator, _graphicsContext.GetFrameContextCount(), {})
            , m_atlasSize(_atlasSize)
            , m_glyphBaseSize(_glyphBaseSize)
            , m_slotFootprints(_allocator, Alignment::NextPowerOfTwo(_atlasSize / _glyphBaseSize))
            , m_slotsBitMap(_allocator, _atlasSize / _glyphBaseSize, 0)
            , m_glyphSlotMap(_allocator)
    {
        const TextureDesc atlasTextureDesc {
            .m_dimensions = { m_atlasSize, m_atlasSize, 1 },
            .m_format = TextureFormat::RGBA8_UNorm,
#if !defined(KE_FINAL)
            .m_debugName = "MSDF font atlas"
#endif
        };

        m_atlasFootprint = _graphicsContext.FetchTextureSubResourcesMemoryFootprints(atlasTextureDesc).front();
        m_atlasTexture = _graphicsContext.CreateTexture({
            .m_desc = atlasTextureDesc,
            .m_footprintPerSubResource = { &m_atlasFootprint, 1 },
            .m_memoryUsage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::SampledImage | MemoryUsage::TransferDstImage,
        });

        m_atlasTextureSubresourceIndex = SubResourceIndexing(atlasTextureDesc, 0);

        u32 currentSize = m_glyphBaseSize;
        for (auto& slotFootprint : m_slotFootprints)
        {
            const TextureDesc desc {
                .m_dimensions = { currentSize, currentSize, 1 },
                .m_format = atlasTextureDesc.m_format,
            };
            slotFootprint = _graphicsContext.FetchTextureSubResourcesMemoryFootprints(desc).front();
            currentSize <<= 1;
        }
    }

    MsdfAtlasManager::~MsdfAtlasManager()
    {}

    MsdfAtlasManager::GlyphRegion MsdfAtlasManager::GetGlyphRegion(Font* _font, u32 _unicodeCodepoint, u8 _sizeLShift)
    {
        const u32 slotsPerRow = m_atlasSize / m_glyphBaseSize;

        GlyphSlot slot {};
        {
            const auto lock = m_lock.AutoLock();
            const auto it = m_glyphSlotMap.find({ _font, _unicodeCodepoint });

            if (it != m_glyphSlotMap.end())
            {
                KE_ASSERT(it->second.m_sizeLShift == _sizeLShift);
                return {
                    .m_x = static_cast<u16>(m_glyphBaseSize * (it->second.m_index % slotsPerRow)),
                    .m_y = static_cast<u16>(m_glyphBaseSize * (it->second.m_index / slotsPerRow)),
                    .m_size = static_cast<u16>(m_glyphBaseSize << it->second.m_sizeLShift),
                    .m_pxRange = static_cast<u16>(4u << it->second.m_sizeLShift),
                };
            }

            const u32 glyphSize = 1 << _sizeLShift;
            const uint2 spot = FindFreeSlot(glyphSize);

            if (spot.x == ~0u)
                return {};

            const u64 bitmask = BitUtils::BitMask<u64>(glyphSize);
            for (u32 y = 0; y < glyphSize; ++y)
            {
                m_slotsBitMap[spot.y + y] |= bitmask << spot.x;
            }

            slot.m_index = slotsPerRow * spot.y + spot.x;
            slot.m_sizeLShift = _sizeLShift;
            m_glyphSlotMap.emplace(GlyphKey { _font, _unicodeCodepoint }, slot);
        }

        m_loadQueue.enqueue({ { _font, _unicodeCodepoint }, nullptr });
        return {
            .m_x = static_cast<u16>(m_glyphBaseSize * (slot.m_index % slotsPerRow)),
            .m_y = static_cast<u16>(m_glyphBaseSize * (slot.m_index / slotsPerRow)),
            .m_size = static_cast<u16>(m_glyphBaseSize << slot.m_sizeLShift),
            .m_pxRange = static_cast<u16>(4u << slot.m_sizeLShift),
        };
    }

    void MsdfAtlasManager::FlushLoads(GraphicsContext& _graphicsContext, CommandListHandle _transfer)
    {
        eastl::vector<GlyphLoadRequest> requests { m_allocator };
        constexpr size_t kMaxRequestsPerFlush = 32;
        requests.resize(kMaxRequestsPerFlush);

        size_t flushCount = m_loadQueue.try_dequeue_bulk(requests.begin(), kMaxRequestsPerFlush);
        if (flushCount == 0)
            return;

        if (GraphicsContext::SupportsNonGlobalBarriers())
        {
            const TextureMemoryBarrier barrier {
                .m_stagesSrc = BarrierSyncStageFlags::All,
                .m_stagesDst = BarrierSyncStageFlags::AllShading,
                .m_accessSrc = BarrierAccessFlags::ShaderResource,
                .m_accessDst = BarrierAccessFlags::TransferDst,
                .m_texture = m_atlasTexture,
                .m_layoutSrc = TextureLayout::ShaderResource,
                .m_layoutDst = TextureLayout::TransferDst,
            };
            _graphicsContext.PlaceMemoryBarriers(_transfer, {}, {}, { &barrier, 1 });
        }

        // Flush all requests into a single vector
        do
        {
            if (flushCount == kMaxRequestsPerFlush)
            {
                const size_t offset = requests.size();
                requests.resize(requests.size() + kMaxRequestsPerFlush);
                flushCount = m_loadQueue.try_dequeue_bulk(requests.begin() + offset, kMaxRequestsPerFlush);
            }
            else
            {
                requests.resize(requests.size() + flushCount - kMaxRequestsPerFlush);
                flushCount = 0;
            }
        }
        while (flushCount > 0);

        u64 cumulatedSize = 0;
        {
            const auto lock = m_lock.AutoLock();
            for (const auto& request : requests)
            {
                const u32 lShift = m_glyphSlotMap[request.m_key].m_sizeLShift;
                const TextureMemoryFootprint& footprint = m_slotFootprints[lShift];
                cumulatedSize += footprint.m_lineByteAlignedSize * footprint.m_height;
            }
        }

        StagingBuffer& stagingBuffer = m_stagingBuffers[_graphicsContext.GetCurrentFrameContextIndex()];

        if (stagingBuffer.m_size < cumulatedSize)
        {
            if (stagingBuffer.m_buffer != GenPool::kInvalidHandle)
                _graphicsContext.DestroyBuffer(stagingBuffer.m_buffer);

            stagingBuffer.m_buffer = _graphicsContext.CreateBuffer({
                .m_desc = {
                    .m_size = cumulatedSize,
#if !defined(KE_FINAL)
                    .m_debugName = eastl::string().sprintf("MSDF font atlas staging buffer %d", _graphicsContext.GetCurrentFrameContextIndex()),
#endif
                },
                .m_usage = MemoryUsage::StageOnce_UsageType | MemoryUsage::TransferSrcBuffer,
            });

            if (GraphicsContext::SupportsNonGlobalBarriers())
            {
                const BufferMemoryBarrier barrier[] = {
                    {
                        .m_stagesSrc = BarrierSyncStageFlags::None,
                        .m_stagesDst = BarrierSyncStageFlags::Transfer,
                        .m_accessSrc = BarrierAccessFlags::None,
                        .m_accessDst = BarrierAccessFlags::TransferSrc,
                        .m_buffer = stagingBuffer.m_buffer,
                    }
                };
                _graphicsContext.PlaceMemoryBarriers(_transfer, {}, barrier, {});
            }

            stagingBuffer.m_size = cumulatedSize;
        }

        BufferMapping mapping { stagingBuffer.m_buffer, cumulatedSize };
        _graphicsContext.MapBuffer(mapping);
        size_t progress = 0;
        const u32 slotsPerRow = m_atlasSize / m_glyphBaseSize;
        for (auto& request : requests)
        {
            GlyphSlot slot {};
            {
                const auto lock = m_lock.AutoLock();
                const auto it = m_glyphSlotMap.find(request.m_key);
                KE_ASSERT(it != m_glyphSlotMap.end());
                slot = it->second;
            }

            const TextureMemoryFootprint& footprint = m_slotFootprints[slot.m_sizeLShift];
            const u64 size = footprint.m_lineByteAlignedSize * footprint.m_height;
            const u32 dims = m_glyphBaseSize << slot.m_sizeLShift;

            if (request.m_buffer == nullptr)
            {
                request.m_buffer = m_allocator.Allocate<float>(dims * dims * 3);
                request.m_key.m_font->GenerateMsdf(
                    request.m_key.m_unicodeCodepoint,
                    dims,
                    4 << slot.m_sizeLShift,
                    { request.m_buffer, sizeof(float) * dims * dims * 3 });
            }

            u32 localProgress = progress;
            for (u32 y = 0; y < dims; ++y)
            {
                auto* pixels = reinterpret_cast<u32*>(mapping.m_ptr + localProgress);
                auto* pixelsEnd = reinterpret_cast<u32*>(mapping.m_ptr + localProgress + footprint.m_lineByteAlignedSize);

                for (u32 x = 0; x < dims; ++x)
                {
                    const Color pixelColor {
                        eastl::clamp(request.m_buffer[y * dims * 3 + 3 * x + 0], 0.f, 1.f),
                        eastl::clamp(request.m_buffer[y * dims * 3 + 3 * x + 1], 0.f, 1.f),
                        eastl::clamp(request.m_buffer[y * dims * 3 + 3 * x + 2], 0.f, 1.f),
                        1.f,
                    };
                    *pixels = pixelColor.ToRgba8();
                    ++pixels;
                }
                if (pixels != pixelsEnd)
                {
                    memset(pixels, 0, (pixelsEnd - pixels) * sizeof(u32));
                }
                localProgress += footprint.m_lineByteAlignedSize;
            }
            m_allocator.deallocate(request.m_buffer);

            const uint3 offset {
                m_glyphBaseSize * (slot.m_index % slotsPerRow),
                m_glyphBaseSize * (slot.m_index / slotsPerRow),
                0
            };
            _graphicsContext.SetTextureRegionData(
                _transfer,
                { .m_size = size, .m_offset = progress, .m_buffer = stagingBuffer.m_buffer },
                m_atlasTexture,
                footprint,
                m_atlasTextureSubresourceIndex,
                offset,
                { dims, dims, 1 });
            progress += size;
        }
        _graphicsContext.UnmapBuffer(mapping);

        if (GraphicsContext::SupportsNonGlobalBarriers())
        {
            const TextureMemoryBarrier barrier {
                .m_stagesSrc = BarrierSyncStageFlags::Transfer,
                .m_stagesDst = BarrierSyncStageFlags::AllShading,
                .m_accessSrc = BarrierAccessFlags::TransferDst,
                .m_accessDst = BarrierAccessFlags::ShaderResource,
                .m_texture = m_atlasTexture,
                .m_layoutSrc = TextureLayout::TransferDst,
                .m_layoutDst = TextureLayout::ShaderResource,
            };
            _graphicsContext.PlaceMemoryBarriers(_transfer, {}, {}, { &barrier, 1 });
        }
    }

    size_t MsdfAtlasManager::GlyphKey::Hasher::operator()(const GlyphKey& _key) const noexcept
    {
        return Hashing::Hash64(&_key, offsetof(GlyphKey, m_unicodeCodepoint) + sizeof(m_unicodeCodepoint));
    }

    uint2 MsdfAtlasManager::FindFreeSlot(u8 _slotSize) const
    {
        const u32 slotsPerRow = m_atlasSize / m_glyphBaseSize;

        // Fast path: slot size is one, we only need to check for the first available bit.
        if (_slotSize == 1)
        {
            const u64 lineBitMask = BitUtils::BitMask<u64>(slotsPerRow);
            for (u32 y = 0; y < m_slotsBitMap.Size(); y++)
            {
                const u64 available = ~m_slotsBitMap[y] & lineBitMask;
                if (available != 0)
                {
                    return { BitUtils::GetLeastSignificantBit(available), y };
                }
            }
            return { ~0u, ~0u };
        }

        u64 horizontal[64];
        const u64 slotBitMask = BitUtils::BitMask<u64>(_slotSize);
        u64 cumulatedBits = 0;
        u32 cumulationStart = 0;

        for (u32 y = 0; y < m_slotsBitMap.Size(); y++)
        {
            horizontal[y] = 0;
            for (u32 x = 0; x <= slotsPerRow - _slotSize; x++)
            {
                if ((m_slotsBitMap[y] & (slotBitMask << x)) == 0)
                    horizontal[y] |= slotBitMask << x;
            }

            if (cumulationStart == y)
            {
                cumulatedBits = horizontal[y];
            }
            else
            {
                cumulatedBits &= horizontal[y];
            }

            if (horizontal[y] == 0)
            {
                cumulationStart = y + 1;
            }
            else if (cumulatedBits == 0)
            {
                cumulatedBits = horizontal[y];
                for (u32 i = y - 1; y > cumulationStart; --y)
                {
                    const u32 testCumulation = horizontal[i] & cumulatedBits;
                    if (testCumulation == 0)
                    {
                        cumulationStart = i + 1;
                        break;
                    }
                    cumulatedBits = testCumulation;
                }
            }
            else if (y - cumulationStart + 1 >= _slotSize)
            {
                return { BitUtils::GetLeastSignificantBit(cumulatedBits), cumulationStart };
            }
        }

        return { ~0u, ~0u };
    }
} // namespace KryneEngine::Modules::TextRendering
