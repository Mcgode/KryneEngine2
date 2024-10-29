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

    private:
        struct AllocationSet
        {
            eastl::vector<MTL::CommandBuffer*> m_usedCommandBuffers;
            bool m_available;
        };

        AllocationSet m_graphicsAllocationSet {};
        AllocationSet m_computeAllocationSet {};
        AllocationSet m_ioAllocationSet {};
    };
} // namespace KryneEngine
