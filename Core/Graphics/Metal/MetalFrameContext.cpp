/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#include "MetalFrameContext.hpp"

#include <Common/Assert.hpp>

namespace KryneEngine
{
    MetalFrameContext::MetalFrameContext(bool _graphicsAvailable, bool _computeAvailable, bool _ioAvailable)
    {
        m_graphicsAllocationSet.m_available = _graphicsAvailable;
        m_computeAllocationSet.m_available = _computeAvailable;
        m_ioAllocationSet.m_available = _ioAvailable;
    }

    MTL::CommandBuffer* MetalFrameContext::BeginGraphicsCommandList(MTL::CommandQueue& _queue)
    {
        KE_ASSERT(m_graphicsAllocationSet.m_available);

        MTL::CommandBuffer* commandBuffer = _queue.commandBuffer();
        KE_ASSERT_FATAL(commandBuffer != nullptr);

        m_graphicsAllocationSet.m_usedCommandBuffers.push_back(commandBuffer);
        return commandBuffer;
    }
} // namespace KryneEngine