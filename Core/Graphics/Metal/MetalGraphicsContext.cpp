/**
 * @file
 * @author Max Godefroy
 * @date 28/10/2024.
 */

#include "MetalGraphicsContext.hpp"

#include <Graphics/Metal/MetalFrameContext.hpp>
#include <Graphics/Metal/MetalSwapChain.hpp>

namespace KryneEngine
{
    void MetalGraphicsContext::EndFrame(u64 _frameId)
    {
        const u8 frameIndex = _frameId % m_frameContextCount;
        MetalFrameContext& frameContext = m_frameContexts[frameIndex];

        if (m_swapChain != nullptr)
        {
            if (frameContext.m_graphicsAllocationSet.m_usedCommandBuffers.empty())
            {
                frameContext.BeginGraphicsCommandList(*m_graphicsQueue);
            }
            frameContext.m_graphicsAllocationSet.m_usedCommandBuffers.back()->presentDrawable(
                m_swapChain->m_drawables[frameIndex].get());
        }

        const auto commitCommandBuffers = [] (MetalFrameContext::AllocationSet& _allocationSet)
        {
            for (auto commandBuffer: _allocationSet.m_usedCommandBuffers)
            {
                commandBuffer->commit();
                commandBuffer->release();
            }
            _allocationSet.m_usedCommandBuffers.clear();
        };

        commitCommandBuffers(frameContext.m_graphicsAllocationSet);
        commitCommandBuffers(frameContext.m_computeAllocationSet);
        commitCommandBuffers(frameContext.m_ioAllocationSet);
    }

    void MetalGraphicsContext::WaitForFrame(u64 _frameId) const
    {}

    bool MetalGraphicsContext::IsFrameExecuted(u64 _frameId) const
    {}
}