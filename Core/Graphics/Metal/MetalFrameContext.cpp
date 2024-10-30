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
        : m_graphicsAllocationSet(_graphicsAvailable)
        , m_computeAllocationSet(_computeAvailable)
        , m_ioAllocationSet(_ioAvailable)
    {}

    MTL::CommandBuffer* MetalFrameContext::BeginGraphicsCommandList(MTL::CommandQueue& _queue)
    {
        KE_ASSERT(m_graphicsAllocationSet.m_available);

        MTL::CommandBuffer* commandBuffer = _queue.commandBuffer();
        KE_ASSERT_FATAL(commandBuffer != nullptr);

        m_graphicsAllocationSet.m_usedCommandBuffers.push_back(commandBuffer);
        return commandBuffer;
    }

    void MetalFrameContext::PrepareForNextFrame(u64 _frameId)
    {
        m_frameId = _frameId;
        m_graphicsAllocationSet.m_committedBuffers = false;
        m_computeAllocationSet.m_committedBuffers = false;
        m_ioAllocationSet.m_committedBuffers = false;
    }

    void MetalFrameContext::WaitForFrame(u64 _frameId)
    {
        if (m_frameId > _frameId)
        {
            return;
        }
        m_graphicsAllocationSet.Wait();
        m_computeAllocationSet.Wait();
        m_ioAllocationSet.Wait();
    }

    MetalFrameContext::AllocationSet::AllocationSet(bool _available)
        : m_available(_available)
    {
        if (_available)
        {
            m_synchronizationSemaphore = dispatch_semaphore_create(0);
        }
    }

    void MetalFrameContext::AllocationSet::Commit()
    {
        if (!m_available)
        {
            return;
        }

        if (!m_usedCommandBuffers.empty())
        {
            m_committedBuffers = true;
            m_usedCommandBuffers.back()->addCompletedHandler(
                [this](MTL::CommandBuffer*){
                    dispatch_semaphore_signal(m_synchronizationSemaphore);
                });
        }

        for (auto commandBuffer: m_usedCommandBuffers)
        {
            commandBuffer->commit();
            commandBuffer->release();
        }
        m_usedCommandBuffers.clear();
    }

    void MetalFrameContext::AllocationSet::Wait()
    {
        if (!m_available)
        {
            return;
        }

        if (m_committedBuffers)
        {
            dispatch_semaphore_wait(m_synchronizationSemaphore, DISPATCH_TIME_FOREVER);
            m_committedBuffers = false;
        }
    }
} // namespace KryneEngine