/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#pragma once

#include "Dx12Headers.hpp"
#include "Dx12FrameContext.hpp"
#include "Dx12Resources.h"
#include "Dx12Types.hpp"
#include <Common/Arrays.hpp>
#include <Graphics/Common/MemoryBarriers.hpp>
#include <Graphics/Common/Texture.hpp>
#include <EASTL/unique_ptr.h>

namespace KryneEngine
{
    class Window;
    class Dx12SwapChain;

    struct BufferCopyParameters;
    struct BufferMapping;
    struct BufferView;
    struct DrawIndexedInstancedDesc;
    struct Viewport;

    class Dx12GraphicsContext
    {
    public:
        explicit Dx12GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo, u64 _currentFrameId);

        ~Dx12GraphicsContext();

        [[nodiscard]] Window* GetWindow() const;

        [[nodiscard]] u8 GetFrameContextCount() const { return m_frameContextCount; }

        void EndFrame(u64 _frameId);

        [[nodiscard]] bool IsFrameExecuted(u64 _frameId) const;

        void WaitForFrame(u64 _frameId) const;

        [[nodiscard]] const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const { return m_appInfo; }

    private:
        GraphicsCommon::ApplicationInfo m_appInfo;

        eastl::unique_ptr<Window> m_window;

        ComPtr<ID3D12Device> m_device;

        ComPtr<ID3D12CommandQueue> m_directQueue;
        ComPtr<ID3D12CommandQueue> m_computeQueue;
        ComPtr<ID3D12CommandQueue> m_copyQueue;

        eastl::unique_ptr<Dx12SwapChain> m_swapChain;

        u8 m_frameContextCount;
        DynamicArray<Dx12FrameContext> m_frameContexts;
        ComPtr<ID3D12Fence> m_frameFence;
        HANDLE m_frameFenceEvent;

        DWORD m_validationLayerMessageCallbackHandle = 0;

        bool m_enhancedBarriersEnabled = false;

        void _CreateDevice(IDXGIFactory4* _factory4);
        void _FindAdapter(IDXGIFactory4* _factory, IDXGIAdapter1** _adapter);

        void _CreateCommandQueues();

    public:
        [[nodiscard]] eastl::vector<TextureMemoryFootprint> FetchTextureSubResourcesMemoryFootprints(const TextureDesc& _desc);

        [[nodiscard]] inline BufferHandle CreateBuffer(const BufferCreateDesc& _desc)
        {
            return m_resources.CreateBuffer(_desc);
        }

        [[nodiscard]] inline BufferHandle CreateStagingBuffer(
            const TextureDesc& _createDesc,
            const eastl::vector<TextureMemoryFootprint>& _footprints)
        {
            return m_resources.CreateStagingBuffer(_createDesc, _footprints);
        }

        [[nodiscard]] bool NeedsStagingBuffer(BufferHandle _buffer);

        inline bool DestroyBuffer(BufferHandle _buffer)
        {
            return m_resources.DestroyBuffer(_buffer);
        }

        [[nodiscard]] TextureHandle CreateTexture(const TextureCreateDesc& _createDesc)
        {
            return m_resources.CreateTexture(_createDesc, m_device.Get());
        }

        inline bool DestroyTexture(TextureHandle _texture)
        {
            return m_resources.ReleaseTexture(_texture, true);
        }

        [[nodiscard]] inline TextureSrvHandle CreateTextureSrv(const TextureSrvDesc& _srvDesc, u64 _frameId)
        {
            return m_resources.CreateTextureSrv(_srvDesc, m_device.Get());
        }

        inline bool DestroyTextureSrv(TextureSrvHandle _textureSrv)
        {
            return m_resources.DestroyTextureSrv(_textureSrv);
        }

        [[nodiscard]] SamplerHandle CreateSampler(const SamplerDesc& _samplerDesc);
        bool DestroySampler(SamplerHandle _sampler);

        [[nodiscard]] inline RenderTargetViewHandle CreateRenderTargetView(const RenderTargetViewDesc& _desc)
        {
            return m_resources.CreateRenderTargetView(_desc, m_device.Get());
        }

        bool DestroyRenderTargetView(RenderTargetViewHandle _rtv)
        {
            return m_resources.FreeRenderTargetView(_rtv);
        }

        [[nodiscard]] RenderTargetViewHandle GetPresentRenderTargetView(u8 _index);
        [[nodiscard]] u32 GetCurrentPresentImageIndex() const;

        RenderPassHandle CreateRenderPass(const RenderPassDesc& _desc)
        {
            return m_resources.CreateRenderPass(_desc);
        }

        bool DestroyRenderPass(RenderPassHandle _renderPass)
        {
            return m_resources.FreeRenderPass(_renderPass);
        }

        CommandList BeginGraphicsCommandList(u64 _frameId);
        void EndGraphicsCommandList(u64 _frameId);

        void BeginRenderPass(CommandList _commandList, RenderPassHandle _renderPass);
        void EndRenderPass(CommandList _commandList);

        void SetTextureData(
            CommandList _commandList,
            BufferHandle _stagingBuffer,
            TextureHandle _dstTexture,
            const TextureMemoryFootprint& _footprint,
            const SubResourceIndexing& _subResourceIndex,
            void* _data);

        void MapBuffer(BufferMapping& _mapping);
        void UnmapBuffer(BufferMapping& _mapping);
        void CopyBuffer(CommandList _commandList, const BufferCopyParameters& _params);

        void PlaceMemoryBarriers(
            CommandList _commandList,
            const eastl::span<GlobalMemoryBarrier>& _globalMemoryBarriers,
            const eastl::span<BufferMemoryBarrier>& _bufferMemoryBarriers,
            const eastl::span<TextureMemoryBarrier>& _textureMemoryBarriers);

        [[nodiscard]] ShaderModuleHandle RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize);
        [[nodiscard]] DescriptorSetHandle CreateDescriptorSet(const DescriptorSetDesc& _desc, u32* _bindingIndices);
        [[nodiscard]] PipelineLayoutHandle CreatePipelineLayout(const PipelineLayoutDesc& _desc);
        [[nodiscard]] GraphicsPipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc);

        void UpdateDescriptorSet(
            DescriptorSetHandle _descriptorSet,
            const eastl::span<DescriptorSetWriteInfo>& _writes,
            u64 _frameId);

        void SetViewport(CommandList  _commandList, const Viewport& _viewport);
        void SetScissorsRect(CommandList  _commandList, const Rect& _rect);
        void SetIndexBuffer(CommandList _commandList, BufferHandle _indexBuffer, u64 _bufferSize, bool _isU16);
        void SetVertexBuffers(CommandList _commandList, const eastl::span<BufferView>& _bufferViews);
        void SetGraphicsPipeline(CommandList _commandList, GraphicsPipelineHandle _graphicsPipeline);
        void SetGraphicsPushConstant(CommandList _commandList, u32 _index, const eastl::span<u32>& _data, u32 _offset = 0);
        void SetGraphicsDescriptorSets(
            CommandList _commandList,
            const eastl::span<DescriptorSetHandle>& _sets,
            const bool* _unchanged,
            u32 _frameId);
        void DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc);

    private:
        Dx12Resources m_resources;
        eastl::unique_ptr<Dx12DescriptorSetManager> m_descriptorSetManager;
        RenderPassHandle m_currentRenderPass;
    };
} // KryneEngine