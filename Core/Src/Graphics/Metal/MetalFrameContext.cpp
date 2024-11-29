/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#include "Graphics/Metal/MetalFrameContext.hpp"

#include "KryneEngine/Core/Common/Assert.hpp"
#include "KryneEngine/Core/Profiling/TracyHeader.hpp"

namespace KryneEngine
{
    MetalFrameContext::MetalFrameContext(
        bool _graphicsAvailable, bool _computeAvailable, bool _ioAvailable, bool _validationLayers)
        : m_graphicsAllocationSet(_graphicsAvailable)
        , m_computeAllocationSet(_computeAvailable)
        , m_ioAllocationSet(_ioAvailable)
        , m_enhancedCommandBufferErrors(_validationLayers)
    {}

    CommandList MetalFrameContext::BeginGraphicsCommandList(MTL::CommandQueue& _queue)
    {
        KE_ASSERT(m_graphicsAllocationSet.m_available);

        KE_AUTO_RELEASE_POOL;
        NsPtr descriptor { MTL::CommandBufferDescriptor::alloc()->init() };
        if (m_enhancedCommandBufferErrors)
        {
            descriptor->setErrorOptions(MTL::CommandBufferErrorOptionEncoderExecutionStatus);
        }
        MTL::CommandBuffer* commandBuffer = _queue.commandBuffer(descriptor.get())->retain();
        KE_ASSERT_FATAL(commandBuffer != nullptr);

        m_graphicsAllocationSet.m_usedCommandBuffers.push_back({ commandBuffer });
        return &m_graphicsAllocationSet.m_usedCommandBuffers.back();
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
        KE_ZoneScopedFunction("MetalFrameContext::WaitForFrame");
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

    void MetalFrameContext::AllocationSet::Commit(bool _enhancedErrors)
    {
        if (!m_available)
        {
            return;
        }

        if (_enhancedErrors)
        {
            for (CommandListData commandList: m_usedCommandBuffers)
            {
                commandList.m_commandBuffer->addCompletedHandler(
                    [](MTL::CommandBuffer* commandBuffer) {
                        {
                            NS::Object* objectPtr = nullptr;
                            NS::FastEnumerationState state {};

                            while (commandBuffer->logs()->countByEnumerating(&state, &objectPtr, 1) != 0)
                            {
                                auto* log = reinterpret_cast<MTL::FunctionLog*>(objectPtr);
                                if (log->debugLocation() != nullptr && log->debugLocation()->functionName() != nullptr)
                                {
                                    KE_ERROR(
                                        "Error at %s:%d:%d (encoder '%s')",
                                        log->debugLocation()->functionName()->cString(NS::UTF8StringEncoding),
                                        log->debugLocation()->line(),
                                        log->debugLocation()->column(),
                                        log->encoderLabel() != nullptr ? log->encoderLabel()->cString(NS::UTF8StringEncoding) : "Unknown label");
                                }
                            }
                        }

                        if (commandBuffer->error() != nullptr)
                        {
                            const char* errorString = commandBuffer->error()->description()->cString(NS::UTF8StringEncoding);
                            KE_ERROR(errorString);
                        }
                    });
            }
        }

        if (!m_usedCommandBuffers.empty())
        {
            m_committedBuffers = true;
            m_usedCommandBuffers.back().m_commandBuffer->addCompletedHandler(
                [this](MTL::CommandBuffer*){
                    dispatch_semaphore_signal(m_synchronizationSemaphore);
                });
        }

        KE_AUTO_RELEASE_POOL;
        for (auto& commandListData : m_usedCommandBuffers)
        {
            if (commandListData.m_encoder != nullptr)
            {
                commandListData.m_encoder->endEncoding();
                commandListData.m_encoder.reset();
            }
            commandListData.m_commandBuffer->commit();
            commandListData.m_commandBuffer->release();
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