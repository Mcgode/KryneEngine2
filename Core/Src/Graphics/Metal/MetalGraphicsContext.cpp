/**
 * @file
 * @author Max Godefroy
 * @date 28/10/2024.
 */

#include "Graphics/Metal/MetalGraphicsContext.hpp"

#include "Graphics/Metal/Helpers/EnumConverters.hpp"
#include "Graphics/Metal/MetalFrameContext.hpp"
#include "Graphics/Metal/MetalSwapChain.hpp"
#include "KryneEngine/Core/Graphics/Drawing.hpp"
#include "KryneEngine/Core/Graphics/GraphicsContext.hpp"
#include "KryneEngine/Core/Profiling/TracyHeader.hpp"

namespace KryneEngine
{
    void MetalGraphicsContext::EndFrame(u64 _frameId)
    {
        KE_ZoneScopedFunction("MetalGraphicsContext::EndFrame");

        // Finish current frame and commit
        {
            KE_ZoneScoped("Finish current frame and commit");

            const u8 frameIndex = _frameId % m_frameContextCount;
            MetalFrameContext& frameContext = m_frameContexts[frameIndex];

            if (m_applicationInfo.m_features.m_present)
            {
                if (frameContext.m_graphicsAllocationSet.m_usedCommandBuffers.empty())
                {
                    KE_ZoneScoped("Begin graphics command buffer for present operation");
                    frameContext.BeginGraphicsCommandList(*m_graphicsQueue);
                }
                m_swapChain.Present(
                    frameContext.m_graphicsAllocationSet.m_usedCommandBuffers.back(),
                    frameIndex);
            }

            {
                KE_ZoneScoped("Commit");
                frameContext.m_graphicsAllocationSet.Commit(frameContext.m_enhancedCommandBufferErrors);
                frameContext.m_computeAllocationSet.Commit(frameContext.m_enhancedCommandBufferErrors);
                frameContext.m_ioAllocationSet.Commit(frameContext.m_enhancedCommandBufferErrors);
            }

            if (m_applicationInfo.m_features.m_present)
            {
                KE_ZoneScoped("Retrieve next drawable");
                m_swapChain.UpdateNextDrawable(frameIndex, m_resources);
            }
        }

        FrameMark;

        // Prepare next frame
        {
            KE_ZoneScoped("Prepare next frame");

            const u64 nextFrame = _frameId + 1;
            const u8 newFrameIndex = nextFrame % m_frameContextCount;

            const u64 previousFrameId = eastl::max<u64>(m_frameContextCount, nextFrame) - m_frameContextCount;
            m_frameContexts[newFrameIndex].WaitForFrame(previousFrameId);

            m_frameContexts[newFrameIndex].PrepareForNextFrame(nextFrame);

            m_argumentBufferManager.UpdateAndFlushArgumentBuffers(m_resources, newFrameIndex);
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

    bool MetalGraphicsContext::HasDedicatedTransferQueue() const
    {
        return m_ioQueue.get() != nullptr;
    }

    bool MetalGraphicsContext::HasDedicatedComputeQueue() const
    {
        return m_computeQueue.get() != nullptr;
    }

    eastl::vector<TextureMemoryFootprint> MetalGraphicsContext::FetchTextureSubResourcesMemoryFootprints(
        const TextureDesc& _desc)
    {
        eastl::vector<TextureMemoryFootprint> result { m_allocator };

        const size_t pixelByteSize = MetalConverters::GetPixelByteSize(_desc.m_format);
        size_t currentOffset = 0;

        for (u16 arraySlice = 0; arraySlice < _desc.m_arraySize; arraySlice++)
        {
            for (u8 mip = 0; mip < _desc.m_mipCount; mip++)
            {
                TextureMemoryFootprint& footprint = result.emplace_back();

                footprint = {
                    .m_offset = currentOffset,
                    .m_width = eastl::max(_desc.m_dimensions.x >> mip, 1u),
                    .m_height = eastl::max(_desc.m_dimensions.y >> mip, 1u),
                    .m_depth = static_cast<u16>(eastl::max(_desc.m_dimensions.z >> mip, 1u)),
                    .m_format = _desc.m_format,
                };

                const size_t rowByteSize = footprint.m_width * pixelByteSize;
                footprint.m_lineByteAlignedSize = rowByteSize;

                currentOffset += rowByteSize * footprint.m_height * footprint.m_depth;
            }
        }

        return result;
    }

    BufferHandle MetalGraphicsContext::CreateBuffer(const BufferCreateDesc& _desc)
    {
        return m_resources.CreateBuffer(*m_device, _desc);
    }

    BufferHandle MetalGraphicsContext::CreateStagingBuffer(
        const TextureDesc& _createDesc,
        const eastl::span<const TextureMemoryFootprint>& _footprints)
    {
        const TextureMemoryFootprint& lastFootprint = _footprints.back();
        const size_t size = lastFootprint.m_offset +
            lastFootprint.m_lineByteAlignedSize * lastFootprint.m_height * lastFootprint.m_depth;

        BufferCreateDesc desc {
            .m_desc = {
                .m_size = size,
#if !defined(KE_FINAL)
                .m_debugName = _createDesc.m_debugName + "/StagingBuffer"
#endif
            },
            .m_usage = MemoryUsage::StageOnce_UsageType | MemoryUsage::TransferSrcBuffer,
        };

        return CreateBuffer(desc);
    }

    bool MetalGraphicsContext::NeedsStagingBuffer(BufferHandle _buffer)
    {
        const MetalResources::BufferColdData* bufferCold = m_resources.m_buffers.GetCold(_buffer.m_handle);
        if (KE_VERIFY(bufferCold != nullptr)) [[likely]]
        {
            return bufferCold->m_options == MTL::ResourceStorageModePrivate;
        }
        return false;
    }

    bool MetalGraphicsContext::DestroyBuffer(BufferHandle _bufferHandle)
    {
        return m_resources.DestroyBuffer(_bufferHandle);
    }

    TextureHandle MetalGraphicsContext::CreateTexture(const TextureCreateDesc& _createDesc)
    {
        return m_resources.CreateTexture(*m_device, _createDesc);
    }

    bool MetalGraphicsContext::DestroyTexture(TextureHandle _handle)
    {
        return m_resources.UnregisterTexture(_handle);
    }

    TextureSrvHandle MetalGraphicsContext::CreateTextureSrv(const TextureSrvDesc& _srvDesc, u64)
    {
        return m_resources.RegisterTextureSrv(_srvDesc);
    }

    bool MetalGraphicsContext::DestroyTextureSrv(TextureSrvHandle _handle)
    {
        return m_resources.UnregisterTextureSrv(_handle);
    }

    SamplerHandle MetalGraphicsContext::CreateSampler(const SamplerDesc& _samplerDesc)
    {
        return m_resources.CreateSampler(*m_device, _samplerDesc);
    }

    bool MetalGraphicsContext::DestroySampler(SamplerHandle _sampler)
    {
        return m_resources.DestroySampler(_sampler);
    }

    BufferCbvHandle MetalGraphicsContext::CreateBufferCbv(const BufferCbvDesc& _cbvDesc)
    {
        return m_resources.RegisterBufferCbv(_cbvDesc);
    }

    bool MetalGraphicsContext::DestroyBufferCbv(BufferCbvHandle _handle)
    {
        return m_resources.UnregisterBufferCbv(_handle);
    }

    RenderTargetViewHandle MetalGraphicsContext::CreateRenderTargetView(const RenderTargetViewDesc& _desc)
    {
        return m_resources.RegisterRtv(_desc);
    }

    bool MetalGraphicsContext::DestroyRenderTargetView(RenderTargetViewHandle _handle)
    {
        return m_resources.UnregisterRtv(_handle);
    }

    RenderTargetViewHandle MetalGraphicsContext::GetPresentRenderTargetView(u8 _swapChainIndex) const
    {
        VERIFY_OR_RETURN(m_applicationInfo.m_features.m_present, { GenPool::kInvalidHandle });

        return m_swapChain.m_rtvs[_swapChainIndex];
    }

    TextureHandle MetalGraphicsContext::GetPresentTexture(u8 _swapChainIndex) const
    {
        VERIFY_OR_RETURN(m_applicationInfo.m_features.m_present, { GenPool::kInvalidHandle });

        return m_swapChain.m_textures[_swapChainIndex];
    }

    u32 MetalGraphicsContext::GetCurrentPresentImageIndex() const
    {
        VERIFY_OR_RETURN(m_applicationInfo.m_features.m_present, 0);
        return m_swapChain.m_index;
    }

    RenderPassHandle MetalGraphicsContext::CreateRenderPass(const RenderPassDesc& _desc)
    {
        return m_resources.CreateRenderPassDescriptor(_desc);
    }

    bool MetalGraphicsContext::DestroyRenderPass(RenderPassHandle _handle)
    {
        return m_resources.DestroyRenderPassDescriptor(_handle);
    }

    void MetalGraphicsContext::BeginRenderPass(CommandList _commandList, RenderPassHandle _handle)
    {
        VERIFY_OR_RETURN_VOID(_commandList != nullptr);

        const MetalResources::RenderPassHotData* rpHot = m_resources.m_renderPasses.Get(_handle.m_handle);
        VERIFY_OR_RETURN_VOID(rpHot != nullptr);

        // Update system RTVs
        for (const auto& systemRtv: rpHot->m_systemRtvs)
        {
            const MetalResources::RtvHotData* rtvHot = m_resources.m_renderTargetViews.Get(systemRtv.m_handle.m_handle);
            VERIFY_OR_RETURN_VOID(rtvHot != nullptr);

            rpHot->m_descriptor->colorAttachments()->object(systemRtv.m_index)
                ->setTexture(rtvHot->m_texture.get());
        }

        // Leaving dangling encoders is expected behaviour.
        // This allows same command type batching, avoiding encoder re-creation
        KE_ASSERT_FATAL(_commandList->m_encoder == nullptr || _commandList->m_type != CommandListData::EncoderType::Render);
        _commandList->ResetEncoder(CommandListData::EncoderType::Render);

        KE_AUTO_RELEASE_POOL;

        _commandList->m_encoder =
            _commandList->m_commandBuffer->renderCommandEncoder(rpHot->m_descriptor.get())->retain();

#if !defined(KE_FINAL)
        auto* string = NS::String::string(rpHot->m_debugName.c_str(), NS::UTF8StringEncoding);
        _commandList->m_encoder->setLabel(string);
#endif

        _commandList->m_userData = m_allocator.Allocate<RenderState>();
    }

    void MetalGraphicsContext::EndRenderPass(CommandList _commandList)
    {
        m_allocator.Delete(static_cast<RenderState*>(_commandList->m_userData));
        _commandList->m_userData = nullptr;
        _commandList->ResetEncoder();
    }

    void MetalGraphicsContext::BeginComputePass(CommandList _commandList)
    {
        KE_ASSERT(_commandList->m_encoder == nullptr || _commandList->m_type == CommandListData::EncoderType::Blit);
        KE_ASSERT(_commandList->m_userData == nullptr);

        KE_AUTO_RELEASE_POOL;

        _commandList->ResetEncoder(CommandListData::EncoderType::Compute);
        _commandList->m_encoder = _commandList->m_commandBuffer->computeCommandEncoder(MTL::DispatchTypeSerial)->retain();
    }

    void MetalGraphicsContext::EndComputePass(CommandList _commandList)
    {
        KE_ASSERT(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Compute);

        _commandList->ResetEncoder();
    }

    void MetalGraphicsContext::SetTextureData(
        CommandList _commandList,
        BufferHandle _stagingBuffer,
        TextureHandle _dstTexture,
        const TextureMemoryFootprint& _footprint,
        const SubResourceIndexing& _subResourceIndex,
        void* _data)
    {
        MTL::Buffer* stagingBuffer = m_resources.m_buffers.Get(_stagingBuffer.m_handle)->m_buffer.get();

        const auto stagingBufferContent = reinterpret_cast<uintptr_t>(stagingBuffer->contents());
        memcpy(
            reinterpret_cast<void*>(stagingBufferContent + _footprint.m_offset),
            _data,
            _footprint.m_lineByteAlignedSize * _footprint.m_height * _footprint.m_depth);

        _commandList->ResetEncoder(CommandListData::EncoderType::Blit);
        if (_commandList->m_encoder == nullptr)
        {
            NsPtr autoReleasePool { NS::AutoreleasePool::alloc()->init() };
            _commandList->m_encoder = _commandList->m_commandBuffer->blitCommandEncoder()->retain();
        }
        auto* encoder = reinterpret_cast<MTL::BlitCommandEncoder*>(_commandList->m_encoder.get());

        encoder->copyFromBuffer(
            stagingBuffer,
            _footprint.m_offset,
            _footprint.m_lineByteAlignedSize,
            _footprint.m_lineByteAlignedSize * _footprint.m_height * _footprint.m_depth,
            MTL::Size { _footprint.m_width, _footprint.m_height, _footprint.m_depth },
            m_resources.m_textures.Get(_dstTexture.m_handle)->m_texture.get(),
            _subResourceIndex.m_arraySlice,
            _subResourceIndex.m_mipIndex,
            MTL::Origin { 0, 0, 0 });
    }

    void MetalGraphicsContext::MapBuffer(BufferMapping& _mapping)
    {
        MTL::Buffer* buffer = m_resources.m_buffers.Get(_mapping.m_buffer.m_handle)->m_buffer.get();

        KE_ASSERT_MSG(_mapping.m_ptr == nullptr, "Did not unmap previous map");

        KE_ASSERT(_mapping.m_size == ~0ull || buffer->length() >= _mapping.m_offset + _mapping.m_size);
        _mapping.m_size = eastl::min(_mapping.m_size, buffer->length() - _mapping.m_offset);

        _mapping.m_ptr = static_cast<std::byte*>(buffer->contents()) + _mapping.m_offset;
    }

    void MetalGraphicsContext::UnmapBuffer(BufferMapping& _mapping)
    {
        auto [hot, cold] = m_resources.m_buffers.GetAll(_mapping.m_buffer.m_handle);
        if ((cold->m_options & MTL::ResourceStorageModeManaged) != 0)
        {
            hot->m_buffer->didModifyRange({_mapping.m_offset, _mapping.m_size});
        }
        _mapping.m_ptr = nullptr;
    }

    void MetalGraphicsContext::CopyBuffer(CommandList _commandList, const BufferCopyParameters& _params)
    {
        _commandList->ResetEncoder(CommandListData::EncoderType::Blit);
        if (_commandList->m_encoder == nullptr)
        {
            NsPtr autoReleasePool { NS::AutoreleasePool::alloc()->init() };
            _commandList->m_encoder = _commandList->m_commandBuffer->blitCommandEncoder()->retain();
        }
        auto* encoder = reinterpret_cast<MTL::BlitCommandEncoder*>(_commandList->m_encoder.get());

        encoder->copyFromBuffer(
            m_resources.m_buffers.Get(_params.m_bufferSrc.m_handle)->m_buffer.get(),
            _params.m_offsetSrc,
            m_resources.m_buffers.Get(_params.m_bufferDst.m_handle)->m_buffer.get(),
            _params.m_offsetDst,
            _params.m_copySize);
    }

    CommandList MetalGraphicsContext::BeginGraphicsCommandList(u64 _frameId)
    {
        VERIFY_OR_RETURN(m_graphicsQueue != nullptr, nullptr);
        const u8 frameIndex = _frameId % m_frameContextCount;
        return m_frameContexts[frameIndex].BeginGraphicsCommandList(*m_graphicsQueue);
    }

    void MetalGraphicsContext::EndGraphicsCommandList(CommandList _commandList, u64 _frameId)
    {
        KE_ASSERT(_commandList != nullptr);
        if (_commandList->m_encoder != nullptr)
        {
            _commandList->m_encoder->endEncoding();
            _commandList->m_encoder = nullptr;
        }
    }

    void MetalGraphicsContext::PlaceMemoryBarriers(
        CommandList _commandList,
        const eastl::span<const GlobalMemoryBarrier>& _globalMemoryBarriers,
        const eastl::span<const BufferMemoryBarrier>& _bufferMemoryBarriers,
        const eastl::span<const TextureMemoryBarrier>& _textureMemoryBarriers)
    {
        const bool isComputePass =  _commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Compute;

        if (!_globalMemoryBarriers.empty())
        {
            KE_ASSERT_FATAL_MSG(isComputePass, "Metal only supports global memory barriers in compute passes");
            auto* encoder = reinterpret_cast<MTL::ComputeCommandEncoder*>(_commandList->m_encoder.get());

            constexpr BarrierAccessFlags bufferAccessFlags =
                  BarrierAccessFlags::VertexBuffer
                | BarrierAccessFlags::IndexBuffer
                | BarrierAccessFlags::ConstantBuffer
                | BarrierAccessFlags::IndirectBuffer
                | BarrierAccessFlags::ShaderResource
                | BarrierAccessFlags::UnorderedAccess
                | BarrierAccessFlags::TransferSrc
                | BarrierAccessFlags::TransferDst
                | BarrierAccessFlags::AccelerationStructureRead
                | BarrierAccessFlags::AccelerationStructureWrite;

            constexpr BarrierAccessFlags textureAccessFlags =
                  BarrierAccessFlags::DepthStencilRead
                | BarrierAccessFlags::ShaderResource
                | BarrierAccessFlags::UnorderedAccess
                | BarrierAccessFlags::TransferSrc
                | BarrierAccessFlags::TransferDst
                | BarrierAccessFlags::ShadingRate;

            constexpr BarrierAccessFlags renderTargetsAccessFlags =
                  BarrierAccessFlags::ColorAttachment
                | BarrierAccessFlags::DepthStencilWrite
                | BarrierAccessFlags::ResolveSrc
                | BarrierAccessFlags::ResolveDst;

            MTL::BarrierScope scope {};
            for (auto& barrier: _globalMemoryBarriers)
            {
                const BarrierAccessFlags accessFlags = barrier.m_accessSrc | barrier.m_accessDst;
                if (BitUtils::EnumHasAny(accessFlags, bufferAccessFlags))
                {
                    scope |= MTL::BarrierScopeBuffers;
                }
                if (BitUtils::EnumHasAny(accessFlags, textureAccessFlags))
                {
                    scope |= MTL::BarrierScopeTextures;
                }
                if (BitUtils::EnumHasAny(accessFlags, renderTargetsAccessFlags))
                {
                    scope |= MTL::BarrierScopeRenderTargets;
                }
            }
            encoder->memoryBarrier(scope);
        }

        eastl::fixed_vector<MTL::Resource*, 32> readStateTransitions;
        eastl::fixed_vector<MTL::Resource*, 32> writeStateTransitions;
        eastl::fixed_vector<MTL::Resource*, 32> readWriteStateTransitions;
        eastl::fixed_vector<MTL::Resource*, 16> memoryBarriers;

        constexpr BarrierAccessFlags readFlags =
              BarrierAccessFlags::AllRead
            & BarrierAccessFlags::VertexBuffer
            & BarrierAccessFlags::IndexBuffer
            & BarrierAccessFlags::ConstantBuffer
            & BarrierAccessFlags::IndirectBuffer
            & BarrierAccessFlags::DepthStencilRead
            & BarrierAccessFlags::ShaderResource
            & BarrierAccessFlags::ResolveSrc
            & BarrierAccessFlags::TransferSrc
            & BarrierAccessFlags::AccelerationStructureRead
            & BarrierAccessFlags::ShadingRate;
        constexpr BarrierAccessFlags writeFlags =
              BarrierAccessFlags::AllWrite
            & BarrierAccessFlags::ColorAttachment
            & BarrierAccessFlags::DepthStencilWrite
            & BarrierAccessFlags::UnorderedAccess
            & BarrierAccessFlags::ResolveDst
            & BarrierAccessFlags::TransferDst
            & BarrierAccessFlags::AccelerationStructureWrite;

        const auto processBarrier = [&]<class T>(T _barrier, MTL::Resource* _resource)
        {
            const bool srcIsRead = BitUtils::EnumHasAny(_barrier.m_accessSrc, readFlags);
            const bool srcIsWrite = BitUtils::EnumHasAny(_barrier.m_accessSrc, writeFlags);
            const bool dstIsRead = BitUtils::EnumHasAny(_barrier.m_accessDst, readFlags);
            const bool dstIsWrite = BitUtils::EnumHasAny(_barrier.m_accessDst, writeFlags);

            if ((srcIsRead != dstIsRead) || (srcIsWrite != dstIsWrite))
            {
                if (dstIsRead)
                {
                    if (dstIsWrite)
                    {
                        readWriteStateTransitions.push_back(_resource);
                    }
                    else
                    {
                        readStateTransitions.push_back(_resource);
                    }
                }
                else
                {
                    writeStateTransitions.push_back(_resource);
                }
            }
            else if (BitUtils::EnumHasAny(_barrier.m_stagesSrc & _barrier.m_stagesDst, BarrierSyncStageFlags::ComputeShading))
            {
                memoryBarriers.push_back(_resource);
            }
        };

        for (const BufferMemoryBarrier& barrier: _bufferMemoryBarriers)
        {
            processBarrier(
                barrier,
                m_resources.m_buffers.Get(barrier.m_buffer.m_handle)->m_buffer.get());
        }

        for (const TextureMemoryBarrier& barrier: _textureMemoryBarriers)
        {
            processBarrier(
                barrier,
                m_resources.m_textures.Get(barrier.m_texture.m_handle)->m_texture.get());
        }

        const auto processTransitions = [&]<class T>(T* _encoder)
        {
            if (!readStateTransitions.empty())
            {
                _encoder->useResources(
                    readStateTransitions.data(),
                    readStateTransitions.size(),
                    MTL::ResourceUsageRead);
            }
            if (!writeStateTransitions.empty())
            {
                _encoder->useResources(
                    writeStateTransitions.data(),
                    writeStateTransitions.size(),
                    MTL::ResourceUsageWrite);
            }
            if (!readWriteStateTransitions.empty())
            {
                _encoder->useResources(
                    readWriteStateTransitions.data(),
                    readWriteStateTransitions.size(),
                    MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
            }
        };

        if (isComputePass)
        {
            auto* encoder = reinterpret_cast<MTL::ComputeCommandEncoder*>(_commandList->m_encoder.get());
            processTransitions(encoder);

            if (!memoryBarriers.empty())
            {
                encoder->memoryBarrier(memoryBarriers.data(), memoryBarriers.size());
            }
        }
        else if (_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render)
        {
            auto* encoder = reinterpret_cast<MTL::RenderCommandEncoder*>(_commandList->m_encoder.get());
            processTransitions(encoder);

            KE_ASSERT_FATAL_MSG(memoryBarriers.empty(), "Metal only supports memory barriers in compute passes");
        }
    }

    void MetalGraphicsContext::DeclarePassTextureSrvUsage(
        CommandList _commandList,
        const eastl::span<const TextureSrvHandle>& _textures)
    {
        KE_ASSERT(_commandList->m_encoder != nullptr
                  && (_commandList->m_type == CommandListData::EncoderType::Render
                      || _commandList->m_type == CommandListData::EncoderType::Compute));

        DynamicArray<MTL::Resource*> resources(m_allocator, _textures.size());

        for (auto i = 0u; i < _textures.size(); ++i)
        {
            resources[i] = m_resources.m_textureSrvs.Get(_textures[i].m_handle)->m_texture.get();
        }

        UseResources(_commandList, { resources.Data(), resources.Size() });
    }

    void MetalGraphicsContext::DeclarePassBufferCbvUsage(
        CommandList _commandList,
        const eastl::span<const BufferCbvHandle>& _buffers)
    {
        KE_ASSERT(_commandList->m_encoder != nullptr
                  && (_commandList->m_type == CommandListData::EncoderType::Render
                      || _commandList->m_type == CommandListData::EncoderType::Compute));

        DynamicArray<MTL::Resource*> resources(m_allocator, _buffers.size());

        for (auto i = 0u; i < _buffers.size(); ++i)
        {
            resources[i] = m_resources.m_bufferCbvs.Get(_buffers[i].m_handle)->m_buffer.get();
        }

        UseResources(_commandList, { resources.Data(), resources.Size() });
    }

    ShaderModuleHandle MetalGraphicsContext::RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize)
    {
        return m_resources.LoadLibrary(*m_device, _bytecodeData, _bytecodeSize);
    }

    DescriptorSetLayoutHandle MetalGraphicsContext::CreateDescriptorSetLayout(
        const DescriptorSetDesc& _desc,
        u32* _bindingIndices)
    {
        return m_argumentBufferManager.CreateArgumentDescriptor(_desc, _bindingIndices);
    }

    DescriptorSetHandle MetalGraphicsContext::CreateDescriptorSet(DescriptorSetLayoutHandle _layout)
    {
        return m_argumentBufferManager.CreateArgumentBuffer(*m_device, _layout);
    }

    PipelineLayoutHandle MetalGraphicsContext::CreatePipelineLayout(const PipelineLayoutDesc& _desc)
    {
        return m_argumentBufferManager.CreatePipelineLayout(_desc);
    }

    GraphicsPipelineHandle MetalGraphicsContext::CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc)
    {
        return m_resources.CreateGraphicsPso(*m_device, m_argumentBufferManager, _desc);
    }

    bool MetalGraphicsContext::DestroyGraphicsPipeline(GraphicsPipelineHandle _pipeline)
    {
        return m_resources.DestroyGraphicsPso(_pipeline);
    }

    bool MetalGraphicsContext::DestroyPipelineLayout(PipelineLayoutHandle _layout)
    {
        return m_argumentBufferManager.DestroyPipelineLayout(_layout);
    }

    bool MetalGraphicsContext::DestroyDescriptorSet(DescriptorSetHandle _set)
    {
        return m_argumentBufferManager.DestroyArgumentBuffer(_set);
    }

    bool MetalGraphicsContext::DestroyDescriptorSetLayout(DescriptorSetLayoutHandle _layout)
    {
        return m_argumentBufferManager.DeleteArgumentDescriptor(_layout);
    }

    bool MetalGraphicsContext::FreeShaderModule(ShaderModuleHandle _module)
    {
        return m_resources.FreeLibrary(_module);
    }

    ComputePipelineHandle MetalGraphicsContext::CreateComputePipeline(const ComputePipelineDesc& _desc)
    {
        return m_resources.CreateComputePso(*m_device, m_argumentBufferManager, _desc);;
    }

    bool MetalGraphicsContext::DestroyComputePipeline(ComputePipelineHandle _pipeline)
    {
        return m_resources.DestroyComputePso(_pipeline);;
    }

    void MetalGraphicsContext::UpdateDescriptorSet(
        DescriptorSetHandle _descriptorSet,
        const eastl::span<const DescriptorSetWriteInfo>& _writes,
        u64 _frameId)
    {
        m_argumentBufferManager.UpdateArgumentBuffer(
            m_resources,
            _writes,
            _descriptorSet,
            _frameId % m_frameContextCount);
    }

    void MetalGraphicsContext::SetViewport(CommandList _commandList, const Viewport& _viewport)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render);

        auto* encoder = reinterpret_cast<MTL::RenderCommandEncoder*>(_commandList->m_encoder.get());
        encoder->setViewport({
            .originX = static_cast<double>(_viewport.m_topLeftX),
            .originY = static_cast<double>(_viewport.m_topLeftY),
            .width = static_cast<double>(_viewport.m_width),
            .height = static_cast<double>(_viewport.m_height),
            .znear = static_cast<double>(_viewport.m_minDepth),
            .zfar = static_cast<double>(_viewport.m_maxDepth),
        });
    }

    void MetalGraphicsContext::SetScissorsRect(CommandList _commandList, const Rect& _rect)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render);
        auto* encoder = reinterpret_cast<MTL::RenderCommandEncoder*>(_commandList->m_encoder.get());

        encoder->setScissorRect({
            .x = _rect.m_left,
            .y = _rect.m_top,
            .width = _rect.m_right - _rect.m_left,
            .height = _rect.m_bottom - _rect.m_top,
        });
    }

    void MetalGraphicsContext::SetIndexBuffer(CommandList _commandList, const BufferView& _indexBufferView, bool _isU16)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render);
        auto* renderState = static_cast<RenderState*>(_commandList->m_userData);
        KE_ASSERT_FATAL(renderState != nullptr);

        renderState->m_indexBufferView = _indexBufferView;
        renderState->m_indexBufferIsU16 = _isU16;
    }

    void MetalGraphicsContext::SetVertexBuffers(CommandList _commandList, const eastl::span<const BufferView>& _bufferViews)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render);
        auto* renderState = static_cast<RenderState*>(_commandList->m_userData);
        KE_ASSERT_FATAL(renderState != nullptr);

        renderState->m_vertexBuffers.clear();
        renderState->m_vertexBuffers.insert(
            renderState->m_vertexBuffers.begin(),
            _bufferViews.begin(),
            _bufferViews.end());
    }

    void MetalGraphicsContext::SetGraphicsPipeline(CommandList _commandList, GraphicsPipelineHandle _graphicsPipeline)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render);
        auto* encoder = reinterpret_cast<MTL::RenderCommandEncoder*>(_commandList->m_encoder.get());
        auto* renderState = static_cast<RenderState*>(_commandList->m_userData);
        KE_ASSERT_FATAL(renderState != nullptr);

        MetalResources::GraphicsPsoHotData* graphicsPsoData = m_resources.m_graphicsPso.Get(_graphicsPipeline.m_handle);

        encoder->setRenderPipelineState(graphicsPsoData->m_pso.get());

        renderState->m_topology = graphicsPsoData->m_topology;
        if (memcmp(&renderState->m_dynamicState, &graphicsPsoData->m_staticState, sizeof(RenderDynamicState)) != 0)
        {
            RenderDynamicState& current = renderState->m_dynamicState;
            const RenderDynamicState& ref = graphicsPsoData->m_staticState;

            if (!graphicsPsoData->m_dynamicBlendFactor && (current.m_blendFactor != ref.m_blendFactor))
            {
                encoder->setBlendColor(
                    ref.m_blendFactor.r,
                    ref.m_blendFactor.g,
                    ref.m_blendFactor.b,
                    ref.m_blendFactor.a);
                current.m_blendFactor = ref.m_blendFactor;
            }

            if (current.m_depthStencilHash != ref.m_depthStencilHash)
            {
                encoder->setDepthStencilState(graphicsPsoData->m_depthStencilState.get());
                current.m_depthStencilHash = ref.m_depthStencilHash;
            }

            if (memcmp(&current.m_depthBias, &ref.m_depthBias, 3 * sizeof(float)) != 0)
            {
                encoder->setDepthBias(ref.m_depthBias, ref.m_depthBiasSlope, ref.m_depthBiasClamp);
                current.m_depthBias = ref.m_depthBias;
                current.m_depthBiasSlope = ref.m_depthBiasSlope;
                current.m_depthBiasClamp = ref.m_depthBiasClamp;
            }

            if (current.m_fillMode != ref.m_fillMode)
            {
                encoder->setTriangleFillMode(MetalConverters::GetTriangleFillMode(ref.m_fillMode));
                current.m_fillMode = ref.m_fillMode;
            }

            if (current.m_cullMode != ref.m_cullMode)
            {
                encoder->setCullMode(MetalConverters::GetCullMode(ref.m_cullMode));
                current.m_cullMode = ref.m_cullMode;
            }

            if (current.m_front != ref.m_front)
            {
                encoder->setFrontFacingWinding(MetalConverters::GetWinding(ref.m_front));
                current.m_front = ref.m_front;
            }

            if (current.m_depthClip != ref.m_depthClip)
            {
                encoder->setDepthClipMode(ref.m_depthClip ? MTL::DepthClipModeClip : MTL::DepthClipModeClamp);
                current.m_depthClip = ref.m_depthClip;
            }

            if (!graphicsPsoData->m_dynamicStencilRef && current.m_stencilRefValue != ref.m_stencilRefValue)
            {
                encoder->setStencilReferenceValue(ref.m_stencilRefValue);
                current.m_stencilRefValue = ref.m_stencilRefValue;
            }
        }

        u8 i = 0;
        for (const BufferView& vertexBufferView: renderState->m_vertexBuffers)
        {
            encoder->setVertexBuffer(
                m_resources.m_buffers.Get(vertexBufferView.m_buffer.m_handle)->m_buffer.get(),
                vertexBufferView.m_offset,
                i + graphicsPsoData->m_vertexBufferFirstIndex);
            i++;
        }
    }

    void MetalGraphicsContext::SetGraphicsPushConstant(
        CommandList _commandList,
        PipelineLayoutHandle _layout,
        const eastl::span<const u32>& _data,
        u32 _index,
        u32 _offset)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render);
        auto* encoder = reinterpret_cast<MTL::RenderCommandEncoder*>(_commandList->m_encoder.get());

        const MetalArgumentBufferManager::PushConstantData& pushConstantData =
            m_argumentBufferManager.m_pipelineLayouts.Get(_layout.m_handle)->m_pushConstantsData[_index];

        for (auto& data: pushConstantData.m_data)
        {
            switch (data.m_visibility)
            {
            case ShaderVisibility::Vertex:
                encoder->setVertexBytes(_data.data(), _data.size() * sizeof(u32), data.m_bufferIndex);
                break;
            case ShaderVisibility::Fragment:
                encoder->setFragmentBytes(_data.data(), _data.size() * sizeof(u32), data.m_bufferIndex);
                break;
            default:
                KE_ERROR("Invalid visibility");
                break;
            }
        }
    }

    void MetalGraphicsContext::SetGraphicsDescriptorSets(
        CommandList _commandList,
        PipelineLayoutHandle _layout,
        const eastl::span<const DescriptorSetHandle>& _sets,
        const bool* _unchanged,
        u64 _frameId)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render);
        auto* encoder = reinterpret_cast<MTL::RenderCommandEncoder*>(_commandList->m_encoder.get());

        const MetalArgumentBufferManager::PipelineLayoutHotData& layoutData =
            *m_argumentBufferManager.m_pipelineLayouts.Get(_layout.m_handle);

        const u8 frameIndex = _frameId % m_frameContextCount;
        u32 i = 0;
        for (DescriptorSetHandle buffer: _sets)
        {
            const ShaderVisibility visibility = layoutData.m_setVisibilities[i];
            const MetalArgumentBufferManager::ArgumentBufferHotData& argBuffer =
                *m_argumentBufferManager.m_argumentBufferSets.Get(buffer.m_handle);

            if (BitUtils::EnumHasAny(visibility, ShaderVisibility::Vertex))
            {
                encoder->setVertexBuffer(
                    argBuffer.m_argumentBuffer.get(),
                    frameIndex * argBuffer.m_encoder->encodedLength(),
                    i);
            }
            if (BitUtils::EnumHasAny(visibility, ShaderVisibility::Fragment))
            {
                encoder->setFragmentBuffer(
                    argBuffer.m_argumentBuffer.get(),
                    frameIndex * argBuffer.m_encoder->encodedLength(),
                    i);
            }

            i++;
        }
    }

    void MetalGraphicsContext::DrawInstanced(CommandList _commandList, const DrawInstancedDesc& _desc)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render);
        auto* encoder = reinterpret_cast<MTL::RenderCommandEncoder*>(_commandList->m_encoder.get());
        auto* renderState = static_cast<RenderState*>(_commandList->m_userData);
        KE_ASSERT_FATAL(renderState != nullptr);

        KE_AUTO_RELEASE_POOL;
        encoder->drawPrimitives(
            MetalConverters::GetPrimitiveType(renderState->m_topology),
            _desc.m_vertexOffset,
            _desc.m_vertexCount,
            _desc.m_instanceCount,
            _desc.m_instanceOffset);
    }

    void MetalGraphicsContext::DrawIndexedInstanced(
        CommandList _commandList,
        const DrawIndexedInstancedDesc& _desc)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Render);
        auto* encoder = reinterpret_cast<MTL::RenderCommandEncoder*>(_commandList->m_encoder.get());
        auto* renderState = static_cast<RenderState*>(_commandList->m_userData);
        KE_ASSERT_FATAL(renderState != nullptr);

        const MTL::IndexType indexType = renderState->m_indexBufferIsU16 ? MTL::IndexTypeUInt16 : MTL::IndexTypeUInt32;
        const size_t indexBufferOffset = renderState->m_indexBufferView.m_offset + _desc.m_indexOffset * (renderState->m_indexBufferIsU16 ? sizeof(u16) : sizeof(u32));

        KE_AUTO_RELEASE_POOL;
        encoder->drawIndexedPrimitives(
            MetalConverters::GetPrimitiveType(renderState->m_topology),
            _desc.m_elementCount,
            indexType,
            m_resources.m_buffers.Get(renderState->m_indexBufferView.m_buffer.m_handle)->m_buffer.get(),
            indexBufferOffset,
            _desc.m_instanceCount,
            _desc.m_vertexOffset,
            _desc.m_instanceOffset);
    }

    void MetalGraphicsContext::SetComputePipeline(CommandList _commandList, ComputePipelineHandle _pipeline)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Compute);
        auto* encoder = reinterpret_cast<MTL::ComputeCommandEncoder*>(_commandList->m_encoder.get());

        MetalResources::ComputePsoHotData* hot = m_resources.m_computePso.Get(_pipeline.m_handle);
        encoder->setComputePipelineState(hot->m_pso.get());
    }

    void MetalGraphicsContext::SetComputeDescriptorSets(
        CommandList _commandList,
        PipelineLayoutHandle _layout,
        eastl::span<const DescriptorSetHandle> _sets,
        u32 _offset,
        u64 _frameId)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Compute);
        auto* encoder = reinterpret_cast<MTL::ComputeCommandEncoder*>(_commandList->m_encoder.get());

        const MetalArgumentBufferManager::PipelineLayoutHotData* layoutData = m_argumentBufferManager.m_pipelineLayouts.Get(_layout.m_handle);
        const u8 frameIndex = _frameId % m_frameContextCount;

        for (u32 i = 0; i < _sets.size(); ++i)
        {
            const u32 index = i + _offset;

            const ShaderVisibility visibility = layoutData->m_setVisibilities[index];
            KE_ASSERT(BitUtils::EnumHasAny(visibility, ShaderVisibility::Compute) || visibility == ShaderVisibility::None);

            const MetalArgumentBufferManager::ArgumentBufferHotData& argBuffer =
                    *m_argumentBufferManager.m_argumentBufferSets.Get(_sets[i].m_handle);

            encoder->setBuffer(
                argBuffer.m_argumentBuffer.get(),
                frameIndex * argBuffer.m_encoder->encodedLength(),
                index);
        }
    }

    void MetalGraphicsContext::SetComputePushConstant(
        CommandList _commandList, PipelineLayoutHandle _layout, eastl::span<const u32> _data)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Compute);
        auto* encoder = reinterpret_cast<MTL::ComputeCommandEncoder*>(_commandList->m_encoder.get());

        const MetalArgumentBufferManager::PushConstantData& pushConstantData =
            m_argumentBufferManager.m_pipelineLayouts.Get(_layout.m_handle)->m_pushConstantsData[0];

        KE_ASSERT(pushConstantData.m_data.size() == 1);
        KE_ASSERT(pushConstantData.m_data[0].m_visibility == ShaderVisibility::Compute);
        encoder->setBytes(_data.data(), _data.size_bytes(), pushConstantData.m_data[0].m_bufferIndex);
    }

    void MetalGraphicsContext::Dispatch(CommandList _commandList, uint3 _threadGroupCount, uint3 _threadGroupSize)
    {
        VERIFY_OR_RETURN_VOID(_commandList->m_encoder != nullptr && _commandList->m_type == CommandListData::EncoderType::Compute);
        auto* encoder = reinterpret_cast<MTL::ComputeCommandEncoder*>(_commandList->m_encoder.get());

        const MTL::Size threadGroupCount {
            _threadGroupCount.x,
            _threadGroupCount.y,
            _threadGroupCount.z
        };

        const MTL::Size threadGroupSize {
            _threadGroupSize.x,
            _threadGroupSize.y,
            _threadGroupSize.z
        };

        encoder->dispatchThreadgroups(threadGroupCount, threadGroupSize);
    }

    void MetalGraphicsContext::UseResources(CommandList _commandList, eastl::span<MTL::Resource*> _resources)
    {
        switch (_commandList->m_type)
        {
        case CommandListData::EncoderType::Render:
        {
            auto* encoder = reinterpret_cast<MTL::RenderCommandEncoder*>(_commandList->m_encoder.get());
            encoder->useResources(_resources.data(), _resources.size(), MTL::ResourceUsageRead);
            break;
        }
        case CommandListData::EncoderType::Compute:
        {
            auto* encoder = reinterpret_cast<MTL::ComputeCommandEncoder*>(_commandList->m_encoder.get());
            encoder->useResources(_resources.data(), _resources.size(), MTL::ResourceUsageRead);
            break;
        }
        default:
            break;
        }
    }
}
