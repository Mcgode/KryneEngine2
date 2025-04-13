/**
 * @file
 * @author Max Godefroy
 * @date 10/03/2025.
 */

#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/span.h>
#include <EASTL/vector.h>
#include <KryneEngine/Core/Graphics/Common/MemoryBarriers.hpp>
#include <KryneEngine/Core/Memory/SimplePool.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    class Builder;
    class Registry;
    struct PassAttachmentDeclaration;

    class ResourceStateTracker
    {
    public:
        void Process(Builder& _builder, Registry& _registry);

        struct PassBarriers
        {
            eastl::span<BufferMemoryBarrier> m_bufferMemoryBarriers {};
            eastl::span<TextureMemoryBarrier> m_textureMemoryBarriers {};
        };

        PassBarriers GetPassBarriers(u32 _passIndex);

    private:
        struct ResourceState
        {
            BarrierSyncStageFlags m_syncStage = BarrierSyncStageFlags::All;
            BarrierAccessFlags m_accessFlags = BarrierAccessFlags::All;
            TextureLayout m_layout = TextureLayout::Unknown;
            bool m_depthPass = false;
            PassAttachmentDeclaration* m_attachment = nullptr;
        };

        struct PassBarriersRaw
        {
            size_t m_bufferMemoryBarriersStart = 0;
            size_t m_bufferMemoryBarriersCount = 0;
            size_t m_textureMemoryBarriersStart = 0;
            size_t m_textureMemoryBarriersCount = 0;
        };

        eastl::vector<BufferMemoryBarrier> m_bufferMemoryBarriers;
        eastl::vector<TextureMemoryBarrier> m_textureMemoryBarriers;
        eastl::vector<PassBarriersRaw> m_passBarriers;

        eastl::hash_map<SimplePoolHandle, ResourceState> m_trackedStates;
    };
} // namespace KryneEngine
