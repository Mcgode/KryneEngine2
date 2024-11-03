/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#pragma once

#include <EASTL/vector.h>
#include <Graphics/Metal/MetalTypes.hpp>

namespace KryneEngine
{
    class MetalFrameContext
    {
        friend class MetalGraphicsContext;

    public:
        MetalFrameContext(bool _graphicsAvailable, bool _computeAvailable, bool _ioAvailable);

        CommandList BeginGraphicsCommandList(MTL::CommandQueue& _queue);

        void PrepareForNextFrame(u64 _frameId);

        void WaitForFrame(u64 _frameId);

    private:
        struct AllocationSet
        {
            eastl::vector<CommandListData> m_usedCommandBuffers {};
            dispatch_semaphore_t m_synchronizationSemaphore = nullptr;
            bool m_available;
            bool m_committedBuffers = false;

            AllocationSet(bool _available);
            void Commit();
            void Wait();
        };

        AllocationSet m_graphicsAllocationSet;
        AllocationSet m_computeAllocationSet;
        AllocationSet m_ioAllocationSet;
        u64 m_frameId;;
    };
} // namespace KryneEngine
