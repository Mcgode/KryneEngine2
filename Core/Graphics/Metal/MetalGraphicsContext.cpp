/**
 * @file
 * @author Max Godefroy
 * @date 28/10/2024.
 */

#include "MetalGraphicsContext.hpp"
#include <Graphics/Common/Drawing.hpp>

#include <Graphics/Metal/Helpers/EnumConverters.hpp>
#include <Graphics/Metal/MetalFrameContext.hpp>
#include <Graphics/Metal/MetalSwapChain.hpp>
#include <Profiling/TracyHeader.hpp>

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

            if (m_swapChain != nullptr)
            {
                if (frameContext.m_graphicsAllocationSet.m_usedCommandBuffers.empty())
                {
                    KE_ZoneScoped("Begin graphics command buffer for present operation");
                    frameContext.BeginGraphicsCommandList(*m_graphicsQueue);
                }
                m_swapChain->Present(
                    &frameContext.m_graphicsAllocationSet.m_usedCommandBuffers.back(),
                    frameIndex);
            }

            {
                KE_ZoneScoped("Commit");
                frameContext.m_graphicsAllocationSet.Commit();
                frameContext.m_computeAllocationSet.Commit();
                frameContext.m_ioAllocationSet.Commit();
            }

            if (m_swapChain)
            {
                KE_ZoneScoped("Retrieve next drawable");
                m_swapChain->UpdateNextDrawable(frameIndex, m_resources);
            }
        }

        FrameMark;

        // Prepare next frame
        {
            KE_ZoneScoped("Prepare next frame");
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

    eastl::vector<TextureMemoryFootprint> MetalGraphicsContext::FetchTextureSubResourcesMemoryFootprints(
        const TextureDesc& _desc)
    {
        eastl::vector<TextureMemoryFootprint> result {};

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
        const eastl::vector<TextureMemoryFootprint>& _footprints)
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
        VERIFY_OR_RETURN(m_swapChain != nullptr, { GenPool::kInvalidHandle });

        return m_swapChain->m_rtvs[_swapChainIndex];
    }

    TextureHandle MetalGraphicsContext::GetPresentTexture(u8 _swapChainIndex) const
    {
        VERIFY_OR_RETURN(m_swapChain != nullptr, { GenPool::kInvalidHandle });

        return m_swapChain->m_textures[_swapChainIndex];
    }

    u32 MetalGraphicsContext::GetCurrentPresentImageIndex() const
    {
        VERIFY_OR_RETURN(m_swapChain != nullptr, 0);
        return m_swapChain->m_index;
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
        _commandList->ResetEncoder(CommandListData::EncoderType::Render);

        _commandList->m_encoder =
            _commandList->m_commandBuffer->renderCommandEncoder(rpHot->m_descriptor.get());
    }

    void MetalGraphicsContext::EndRenderPass(CommandList _commandList)
    {
        _commandList->ResetEncoder();
    }

    CommandList MetalGraphicsContext::BeginGraphicsCommandList(u64 _frameId)
    {
        VERIFY_OR_RETURN(m_graphicsQueue != nullptr, nullptr);
        const u8 frameIndex = _frameId % m_frameContextCount;
        return m_frameContexts[frameIndex].BeginGraphicsCommandList(*m_graphicsQueue);
    }

    void MetalGraphicsContext::EndGraphicsCommandList(u64 _frameId)
    {}

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
}