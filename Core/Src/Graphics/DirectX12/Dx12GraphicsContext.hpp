/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#pragma once

#include <EASTL/unique_ptr.h>

#include "Graphics/DirectX12/Dx12DescriptorSetManager.hpp"
#include "Graphics/DirectX12/Dx12FrameContext.hpp"
#include "Graphics/DirectX12/Dx12Headers.hpp"
#include "Graphics/DirectX12/Dx12Resources.h"
#include "Graphics/DirectX12/Dx12SwapChain.hpp"
#include "Graphics/DirectX12/Dx12Types.hpp"
#include "KryneEngine/Core/Graphics/Common/MemoryBarriers.hpp"
#include "KryneEngine/Core/Graphics/Common/Texture.hpp"
#include "KryneEngine/Core/Memory/DynamicArray.hpp"

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
        explicit Dx12GraphicsContext(
            AllocatorInstance _allocator,
            const GraphicsCommon::ApplicationInfo& _appInfo,
            Window* _window,
            u64 _currentFrameId);

        ~Dx12GraphicsContext();

        [[nodiscard]] u8 GetFrameContextCount() const { return m_frameContextCount; }

        void EndFrame(u64 _frameId);

        [[nodiscard]] bool IsFrameExecuted(u64 _frameId) const;

        void WaitForFrame(u64 _frameId) const;

        [[nodiscard]] const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const { return m_appInfo; }

        [[nodiscard]] bool HasDedicatedTransferQueue() const;
        [[nodiscard]] bool HasDedicatedComputeQueue() const;

    private:
        AllocatorInstance m_allocator;
        GraphicsCommon::ApplicationInfo m_appInfo;

        ComPtr<ID3D12Device> m_device;

        ComPtr<ID3D12CommandQueue> m_directQueue;
        ComPtr<ID3D12CommandQueue> m_computeQueue;
        ComPtr<ID3D12CommandQueue> m_copyQueue;

        Dx12SwapChain m_swapChain;

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
            const eastl::span<const TextureMemoryFootprint>& _footprints)
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
        [[nodiscard]] TextureHandle GetPresentTexture(u8 _swapChainIndex);
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
            const eastl::span<const GlobalMemoryBarrier>& _globalMemoryBarriers,
            const eastl::span<const BufferMemoryBarrier>& _bufferMemoryBarriers,
            const eastl::span<const TextureMemoryBarrier>& _textureMemoryBarriers);

        void DeclarePassTextureSrvUsage(CommandList _commandList, const eastl::span<const TextureSrvHandle>& _textures) {}

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
            const eastl::span<const DescriptorSetWriteInfo>& _writes,
            u64 _frameId);

        void SetViewport(CommandList  _commandList, const Viewport& _viewport);
        void SetScissorsRect(CommandList  _commandList, const Rect& _rect);
        void SetIndexBuffer(CommandList _commandList, const BufferView& _indexBufferView, bool _isU16);
        void SetVertexBuffers(CommandList _commandList, const eastl::span<const BufferView>& _bufferViews);
        void SetGraphicsPipeline(CommandList _commandList, GraphicsPipelineHandle _graphicsPipeline);
        void SetGraphicsPushConstant(
            CommandList _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<const u32>& _data,
            u32 _index,
            u32 _offset);
        void SetGraphicsDescriptorSets(
            CommandList _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<const DescriptorSetHandle>& _sets,
            const bool* _unchanged,
            u32 _frameId);
        void DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc);

    private:
        Dx12Resources m_resources;
        Dx12DescriptorSetManager m_descriptorSetManager;
        RenderPassHandle m_currentRenderPass;
    };
} // KryneEngine