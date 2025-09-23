/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#pragma once

#include <atomic>
#include <EASTL/vector.h>

#include "KryneEngine/Core/Memory/DynamicArray.hpp"
#include "Graphics/Metal/MetalTypes.hpp"

namespace KryneEngine
{
    class MetalFrameContext
    {
        friend class MetalGraphicsContext;

    public:
        MetalFrameContext(
            MTL::Device* _device,
            AllocatorInstance _allocator,
            u32 _timestampCount,
            bool _graphicsAvailable,
            bool _computeAvailable,
            bool _ioAvailable,
            bool _validationLayers);

        CommandList BeginGraphicsCommandList(MTL::CommandQueue& _queue);

        void PrepareForNextFrame(u64 _frameId);

        void WaitForFrame(u64 _frameId);

        void ResolveCounters(const TimestampConversion& _conversion);

        u32 AllocateTimestamp();

    private:
        struct AllocationSet
        {
            eastl::vector<CommandListData*> m_usedCommandBuffers {};
            dispatch_semaphore_t m_synchronizationSemaphore = nullptr;
            bool m_available;
            bool m_committedBuffers = false;

            AllocationSet(AllocatorInstance _allocator, bool _available);
            void Commit(bool _enhancedErrors);
            void Wait();
        };

        AllocationSet m_graphicsAllocationSet;
        AllocationSet m_computeAllocationSet;
        AllocationSet m_ioAllocationSet;
        u64 m_frameId;
        bool m_enhancedCommandBufferErrors;

        NsPtr<MTL::CounterSampleBuffer> m_sampleBuffer;
        DynamicArray<u64> m_resolvedTimestamps;
        std::atomic<u32> m_timestampIndex;
        u64 m_lastResolvedFrame = ~0ull;
    };
} // namespace KryneEngine
