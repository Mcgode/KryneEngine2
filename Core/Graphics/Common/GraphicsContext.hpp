/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#if defined(KE_GRAPHICS_API_VK)
#   include <Graphics/VK/VkGraphicsContext.hpp>
#elif defined(KE_GRAPHICS_API_DX12)
#   include <Graphics/DX12/Dx12GraphicsContext.hpp>
#else
#   error No valid graphics API
#endif

namespace KryneEngine
{
    class Window;

    class GraphicsContext
    {
    public:
        explicit GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo);

        ~GraphicsContext();

        [[nodiscard]] inline Window* GetWindow() const
        {
            return m_implementation.GetWindow();
        }

        [[nodiscard]] inline u64 GetFrameId() const
        {
            return m_frameId;
        }

        [[nodiscard]] inline u8 GetFrameContextCount() const
        {
            return m_implementation.GetFrameContextCount();
        }

        [[nodiscard]] inline u8 GetCurrentFrameContextIndex() const
        {
            return m_frameId % GetFrameContextCount();
        }

        bool EndFrame();

        void WaitForLastFrame() const;

        [[nodiscard]] inline bool IsFrameExecuted(u64 _frameId) const
        {
            return m_implementation.IsFrameExecuted(_frameId);
        }

        [[nodiscard]] inline const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const
        {
            return m_implementation.GetApplicationInfo();
        }

        [[nodiscard]] static const char* GetShaderFileExtension();

    private:
#if defined(KE_GRAPHICS_API_VK)
        using UnderlyingGraphicsContext = VkGraphicsContext;
#elif defined(KE_GRAPHICS_API_DX12)
        using UnderlyingGraphicsContext = Dx12GraphicsContext;
#endif
        UnderlyingGraphicsContext m_implementation;

        static constexpr u64 kInitialFrameId = 1;
        u64 m_frameId;

    public:
        [[nodiscard]] eastl::vector<TextureMemoryFootprint> FetchTextureSubResourcesMemoryFootprints(const TextureDesc& _desc)
        {
            return m_implementation.FetchTextureSubResourcesMemoryFootprints(_desc);
        }

        [[nodiscard]] inline BufferHandle CreateBuffer(const BufferCreateDesc& _desc)
        {
            return m_implementation.CreateBuffer(_desc);
        }

        [[nodiscard]] inline BufferHandle CreateStagingBuffer(
            const TextureDesc& _createDesc,
            const eastl::vector<TextureMemoryFootprint>& _footprints)
        {
            return m_implementation.CreateStagingBuffer(_createDesc, _footprints);
        }

        [[nodiscard]] inline bool NeedsStagingBuffer(BufferHandle _buffer)
        {
            return m_implementation.NeedsStagingBuffer(_buffer);
        }

        inline bool DestroyBuffer(BufferHandle _bufferHandle)
        {
            return m_implementation.DestroyBuffer(_bufferHandle);
        }

        [[nodiscard]] inline TextureHandle CreateTexture(const TextureCreateDesc& _createDesc)
        {
            if (!KE_VERIFY_MSG(
                    BitUtils::EnumHasAll(_createDesc.m_memoryUsage, MemoryUsage::GpuOnly_UsageType),
                    "The engine is designed around having buffers representing textures on the CPU")) [[unlikely]]
            {
                return { GenPool::kInvalidHandle };
            }

            return m_implementation.CreateTexture(_createDesc);
        }

        inline bool DestroyTexture(TextureHandle _handle)
        {
            return m_implementation.DestroyTexture(_handle);
        }

        [[nodiscard]] TextureSrvHandle CreateTextureSrv(const TextureSrvDesc& _srvDesc)
        {
            return m_implementation.CreateTextureSrv(_srvDesc, m_frameId);
        }

        inline bool DestroyTextureSrv(TextureSrvHandle _handle)
        {
            return m_implementation.DestroyTextureSrv(_handle);
        }

        [[nodiscard]] SamplerHandle CreateSampler(const SamplerDesc& _samplerDesc);
        bool DestroySampler(SamplerHandle _sampler);

        [[nodiscard]] RenderTargetViewHandle CreateRenderTargetView(const RenderTargetViewDesc& _desc)
        {
            return m_implementation.CreateRenderTargetView(_desc);
        }

        bool DestroyRenderTargetView(RenderTargetViewHandle _handle)
        {
            return m_implementation.DestroyRenderTargetView(_handle);
        }

        [[nodiscard]] RenderTargetViewHandle GetPresentRenderTargetView(u8 _swapChainIndex)
        {
            return m_implementation.GetPresentRenderTargetView(_swapChainIndex);
        }

        [[nodiscard]] inline u32 GetCurrentPresentImageIndex() const
        {
            return m_implementation.GetCurrentPresentImageIndex();
        }

        [[nodiscard]] RenderPassHandle CreateRenderPass(const RenderPassDesc& _desc)
        {
            return m_implementation.CreateRenderPass(_desc);
        }

        bool DestroyRenderPass(RenderPassHandle _handle)
        {
            return m_implementation.DestroyRenderPass(_handle);
        }

        CommandList BeginGraphicsCommandList()
        {
            return m_implementation.BeginGraphicsCommandList(m_frameId);
        }

        void EndGraphicsCommandList()
        {
            m_implementation.EndGraphicsCommandList(m_frameId);
        }

        void BeginRenderPass(CommandList _commandList, RenderPassHandle _handle)
        {
            m_implementation.BeginRenderPass(_commandList, _handle);
        }

        void EndRenderPass(CommandList _commandList)
        {
            m_implementation.EndRenderPass(_commandList);
        }

        inline void SetTextureData(
            CommandList _commandList,
            BufferHandle _stagingBuffer,
            TextureHandle _dstTexture,
            const TextureMemoryFootprint& _footprint,
            const SubResourceIndexing& _subResourceIndex,
            void* _data)
        {
            m_implementation.SetTextureData(
                _commandList,
                _stagingBuffer,
                _dstTexture,
                _footprint,
                _subResourceIndex,
                _data);
        }

        inline void MapBuffer(BufferMapping& _mapping)
        {
            m_implementation.MapBuffer(_mapping);
        }

        inline void UnmapBuffer(BufferMapping& _mapping)
        {
            m_implementation.UnmapBuffer(_mapping);
        }

        inline void CopyBuffer(CommandList _commandList, const BufferCopyParameters& _params)
        {
            m_implementation.CopyBuffer(_commandList, _params);
        }

        inline void PlaceMemoryBarriers(
            CommandList _commandList,
            const eastl::span<GlobalMemoryBarrier>& _globalMemoryBarriers,
            const eastl::span<BufferMemoryBarrier>& _bufferMemoryBarriers,
            const eastl::span<TextureMemoryBarrier>& _textureMemoryBarriers)
        {
            m_implementation.PlaceMemoryBarriers(
                _commandList,
                _globalMemoryBarriers,
                _bufferMemoryBarriers,
                _textureMemoryBarriers);
        }

        [[nodiscard]] ShaderModuleHandle RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize);
        [[nodiscard]] DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetDesc& _desc, u32* _bindingIndices);
        [[nodiscard]] DescriptorSetHandle CreateDescriptorSet(DescriptorSetLayoutHandle _layout);
        [[nodiscard]] PipelineLayoutHandle CreatePipelineLayout(const PipelineLayoutDesc& _desc);
        [[nodiscard]] GraphicsPipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc);
        bool DestroyGraphicsPipeline(GraphicsPipelineHandle _pipeline);
        bool DestroyPipelineLayout(PipelineLayoutHandle _layout);
        bool DestroyDescriptorSet(DescriptorSetHandle _set);
        bool DestroyDescriptorSetLayout(DescriptorSetLayoutHandle _layout);
        bool FreeShaderModule(ShaderModuleHandle _module);

        void UpdateDescriptorSet(
            DescriptorSetHandle _descriptorSet,
            const eastl::span<DescriptorSetWriteInfo>& _writes);

        void SetViewport(CommandList  _commandList, const Viewport& _viewport);
        void SetScissorsRect(CommandList _commandList, const Rect& _rect);
        void SetIndexBuffer(CommandList _commandList, const BufferView& _indexBufferView, bool _isU16 = false);
        void SetVertexBuffers(CommandList _commandList, const eastl::span<BufferView>& _bufferViews);
        void SetGraphicsPipeline(CommandList _commandList, GraphicsPipelineHandle _graphicsPipeline);
        void SetGraphicsPushConstant(
            CommandList _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<u32>& _data,
            u32 _index = 0,
            u32 _offset = 0);
        void SetGraphicsDescriptorSets(
            CommandList _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<DescriptorSetHandle>& _sets,
            const bool* _unchanged = nullptr);
        void DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc);
    };
}


