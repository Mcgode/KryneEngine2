/**
 * @file
 * @author Max Godefroy
 * @date 28/10/2024.
 */

#pragma once

#include <EASTL/unique_ptr.h>

#include "Graphics/Metal/MetalArgumentBufferManager.hpp"
#include "Graphics/Metal/MetalHeaders.hpp"
#include "Graphics/Metal/MetalResources.hpp"
#include "Graphics/Metal/MetalTypes.hpp"
#include "KryneEngine/Core/Graphics/Common/Buffer.hpp"
#include "KryneEngine/Core/Graphics/Common/GraphicsCommon.hpp"
#include "KryneEngine/Core/Graphics/Common/Handles.hpp"
#include "KryneEngine/Core/Graphics/Common/MemoryBarriers.hpp"
#include "KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp"
#include "KryneEngine/Core/Graphics/Common/Texture.hpp"

namespace KryneEngine
{
    struct DrawIndexedInstancedDesc;
    struct RenderPassDesc;
    struct RenderTargetViewDesc;
    struct TextureCreateDesc;
    struct TextureSrvDesc;
    struct Viewport;

    class MetalFrameContext;
    class MetalSwapChain;
    class Window;

    class MetalGraphicsContext
    {
    public:
        MetalGraphicsContext(
            const GraphicsCommon::ApplicationInfo& _appInfo,
            const Window* _window,
            u64 _initialFrameId);

        ~MetalGraphicsContext();

        [[nodiscard]] u8 GetFrameContextCount() const { return m_frameContextCount; }

        void EndFrame(u64 _frameId);
        void WaitForFrame(u64 _frameId) const;
        [[nodiscard]] bool IsFrameExecuted(u64 _frameId) const;

        [[nodiscard]] const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const { return m_applicationInfo; }

    private:
        const GraphicsCommon::ApplicationInfo m_applicationInfo;
        NsPtr<MTL::Device> m_device;
        eastl::unique_ptr<MetalSwapChain> m_swapChain;

        NsPtr<MTL::CommandQueue> m_graphicsQueue;
        NsPtr<MTL::CommandQueue> m_computeQueue;
        NsPtr<MTL::IOCommandQueue> m_ioQueue;

        u8 m_frameContextCount;
        DynamicArray<MetalFrameContext> m_frameContexts;

    public:
        [[nodiscard]] eastl::vector<TextureMemoryFootprint> FetchTextureSubResourcesMemoryFootprints(const TextureDesc& _desc);

        [[nodiscard]] BufferHandle CreateBuffer(const BufferCreateDesc& _desc);
        [[nodiscard]] BufferHandle CreateStagingBuffer(
            const TextureDesc& _createDesc,
            const eastl::vector<TextureMemoryFootprint>& _footprints);
        [[nodiscard]] bool NeedsStagingBuffer(BufferHandle _buffer);
        bool DestroyBuffer(BufferHandle _bufferHandle);

        [[nodiscard]] TextureHandle CreateTexture(const TextureCreateDesc& _createDesc);
        bool DestroyTexture(TextureHandle _handle);

        [[nodiscard]] TextureSrvHandle CreateTextureSrv(const TextureSrvDesc& _srvDesc, u64 _frameId);
        bool DestroyTextureSrv(TextureSrvHandle _handle);

        [[nodiscard]] SamplerHandle CreateSampler(const SamplerDesc& _samplerDesc);
        bool DestroySampler(SamplerHandle _sampler);

        [[nodiscard]] RenderTargetViewHandle CreateRenderTargetView(const RenderTargetViewDesc& _desc);
        bool DestroyRenderTargetView(RenderTargetViewHandle _handle);

        [[nodiscard]] RenderTargetViewHandle GetPresentRenderTargetView(u8 _swapChainIndex) const;
        [[nodiscard]] TextureHandle GetPresentTexture(u8 _swapChainIndex) const;
        [[nodiscard]] u32 GetCurrentPresentImageIndex() const;

        [[nodiscard]] RenderPassHandle CreateRenderPass(const RenderPassDesc& _desc);
        bool DestroyRenderPass(RenderPassHandle _handle);

        CommandList BeginGraphicsCommandList(u64 _frameId);
        void EndGraphicsCommandList(u64 _frameId);

        void BeginRenderPass(CommandList _commandList, RenderPassHandle _handle);
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

        void DeclarePassTextureSrvUsage(CommandList _commandList, const eastl::span<TextureSrvHandle>& _textures);

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
            const eastl::span<DescriptorSetWriteInfo>& _writes,
            u64 _frameId);

        void SetViewport(CommandList _commandList, const Viewport& _viewport);
        void SetScissorsRect(CommandList _commandList, const Rect& _rect);
        void SetIndexBuffer(CommandList _commandList, const BufferView& _indexBufferView, bool _isU16);
        void SetVertexBuffers(CommandList _commandList, const eastl::span<BufferView>& _bufferViews);
        void SetGraphicsPipeline(CommandList _commandList, GraphicsPipelineHandle _graphicsPipeline);
        void SetGraphicsPushConstant(
            CommandList _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<u32>& _data,
            u32 _index,
            u32 _offset);
        void SetGraphicsDescriptorSets(
            CommandList _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<DescriptorSetHandle>& _sets,
            const bool* _unchanged,
            u64 _frameId);
        void DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc);

    private:
        MetalResources m_resources;
        MetalArgumentBufferManager m_argumentBufferManager;
    };
} // KryneEngine