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
        // Finish current frame and commit
        {
            const u8 frameIndex = _frameId % m_frameContextCount;
            MetalFrameContext& frameContext = m_frameContexts[frameIndex];

            if (m_swapChain != nullptr)
            {
                if (frameContext.m_graphicsAllocationSet.m_usedCommandBuffers.empty())
                {
                    frameContext.BeginGraphicsCommandList(*m_graphicsQueue);
                }
                m_swapChain->Present(frameContext.m_graphicsAllocationSet.m_usedCommandBuffers.back(), frameIndex);
            }

            frameContext.m_graphicsAllocationSet.Commit();
            frameContext.m_computeAllocationSet.Commit();
            frameContext.m_ioAllocationSet.Commit();

            if (m_swapChain)
            {
                m_swapChain->UpdateNextDrawable(frameIndex);
            }
        }

        // Prepare next frame
        {
            const u64 nextFrame = _frameId;
            const u8 newFrameIndex = nextFrame % m_frameContextCount;

            const u64 previousFrameId = eastl::max<u64>(m_frameContextCount, nextFrame) - m_frameContextCount;
            m_frameContexts[newFrameIndex].WaitForFrame(previousFrameId);

            m_frameContexts[newFrameIndex].PrepareForNextFrame(nextFrame);
        }
    }

    void MetalGraphicsContext::WaitForFrame(u64 _frameId) const
    {
        for (auto& frameContext: m_frameContexts)
        {
            frameContext.WaitForFrame(_frameId);
        }
    }

    bool MetalGraphicsContext::IsFrameExecuted(u64 _frameId) const
    {
        const u8 frameIndex = _frameId % m_frameContextCount;
        return _frameId < m_frameContexts[frameIndex].m_frameId;
    }
}