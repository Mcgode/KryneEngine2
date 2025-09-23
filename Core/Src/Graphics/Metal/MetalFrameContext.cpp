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
        MTL::Device* _device,
        AllocatorInstance _allocator,
        u32 _timestampCount,
        bool _graphicsAvailable,
        bool _computeAvailable,
        bool _ioAvailable,
        bool _validationLayers)
        : m_graphicsAllocationSet(_allocator, _graphicsAvailable)
        , m_computeAllocationSet(_allocator, _computeAvailable)
        , m_ioAllocationSet(_allocator, _ioAvailable)
        , m_enhancedCommandBufferErrors(_validationLayers)
    {
        if (_timestampCount > 0)
        {
            {
                KE_AUTO_RELEASE_POOL;
                MTL::CounterSampleBufferDescriptor* descriptor = MTL::CounterSampleBufferDescriptor::alloc()->init();

                for (auto i = 0; i < _device->counterSets()->count(); i++)
                {
                    auto* counterSet = reinterpret_cast<MTL::CounterSet*>(_device->counterSets()->object(i));
                    if (counterSet->name()->isEqualToString(MTL::CommonCounterSetTimestamp))
                    {
                        descriptor->setCounterSet(counterSet);
                        break;
                    }
                }
                KE_ASSERT_FATAL(descriptor->counterSet() != nullptr);

                descriptor->setStorageMode(MTL::StorageMode::StorageModeShared);
                descriptor->setSampleCount(_timestampCount);
#if !defined(KE_FINAL)
                descriptor->setLabel(NS::String::string("TimestampBuffer", NS::UTF8StringEncoding));
#endif
                NS::Error* error = nullptr;
                m_sampleBuffer = _device->newCounterSampleBuffer(descriptor, &error);
                KE_ASSERT_FATAL_MSG(m_sampleBuffer != nullptr, error->localizedDescription()->cString(NS::UTF8StringEncoding));
            }

            m_resolvedTimestamps.SetAllocator(_allocator);
            m_resolvedTimestamps.Resize(_timestampCount);
        }
    }

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

        auto* commandList = m_graphicsAllocationSet.m_usedCommandBuffers.get_allocator().New<CommandListData>(commandBuffer);
        m_graphicsAllocationSet.m_usedCommandBuffers.push_back(commandList);
        return commandList;
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

    void MetalFrameContext::ResolveCounters(const TimestampConversion& _conversion)
    {
        if (m_sampleBuffer == nullptr)
            return;

        if (m_lastResolvedFrame == m_frameId)
            return;
        m_lastResolvedFrame = m_frameId;

        const u32 sampleCount = m_timestampIndex.load(std::memory_order_acquire);
        if (sampleCount == 0)
            return;

        m_timestampIndex.store(0, std::memory_order_release);

        KE_AUTO_RELEASE_POOL;
        const NS::Data* resolvedNsData = m_sampleBuffer->resolveCounterRange(NS::Range(0, sampleCount));
        KE_ASSERT(resolvedNsData != nullptr);
        KE_ASSERT(resolvedNsData->length() == sampleCount * sizeof(MTL::CounterResultTimestamp));

        const auto* resolvedTimestamps = reinterpret_cast<const MTL::CounterResultTimestamp*>(resolvedNsData->mutableBytes());
        for (u32 i = 0; i < sampleCount; ++i)
        {
            m_resolvedTimestamps[i] = _conversion.ConvertGpuTimestamp(resolvedTimestamps[i].timestamp);
        }
    }

    u32 MetalFrameContext::AllocateTimestamp()
    {
        return m_timestampIndex.fetch_add(1, std::memory_order_acquire);
    }

    MetalFrameContext::AllocationSet::AllocationSet(AllocatorInstance _allocator, bool _available)
        : m_usedCommandBuffers(_allocator)
        , m_available(_available)
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
            for (CommandListData* commandList: m_usedCommandBuffers)
            {
                commandList->m_commandBuffer->addCompletedHandler(
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
            m_usedCommandBuffers.back()->m_commandBuffer->addCompletedHandler(
                [this](MTL::CommandBuffer*){
                    dispatch_semaphore_signal(m_synchronizationSemaphore);
                });
        }

        KE_AUTO_RELEASE_POOL;
        for (auto& commandListData : m_usedCommandBuffers)
        {
            KE_ASSERT(commandListData->m_encoder == nullptr);
            commandListData->m_commandBuffer->commit();
            commandListData->m_commandBuffer->release();
            m_usedCommandBuffers.get_allocator().deallocate(commandListData);
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