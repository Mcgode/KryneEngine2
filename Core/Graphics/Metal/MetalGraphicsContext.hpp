/**
 * @file
 * @author Max Godefroy
 * @date 28/10/2024.
 */

#pragma once

#include "MetalResources.hpp"
#include <EASTL/unique_ptr.h>
#include <Graphics/Common/Buffer.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Graphics/Common/Handles.hpp>
#include <Graphics/Common/MemoryBarriers.hpp>
#include <Graphics/Common/ShaderPipeline.hpp>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/Metal/MetalHeaders.hpp>
#include <Graphics/Metal/MetalTypes.hpp>

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
        [[nodiscard]] eastl::vector<TextureMemoryFootprint> FetchTextureSubResourcesMemoryFootprints(const TextureDesc& _desc) { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] BufferHandle CreateBuffer(const BufferCreateDesc& _desc) { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] BufferHandle CreateStagingBuffer(
            const TextureDesc& _createDesc,
            const eastl::vector<TextureMemoryFootprint>& _footprints) { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] bool NeedsStagingBuffer(BufferHandle _buffer) { KE_ERROR("NYI"); return {}; }

        bool DestroyBuffer(BufferHandle _bufferHandle) { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] TextureHandle CreateTexture(const TextureCreateDesc& _createDesc) { KE_ERROR("NYI"); return {}; }

        bool DestroyTexture(TextureHandle _handle) { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] TextureSrvHandle CreateTextureSrv(const TextureSrvDesc& _srvDesc, u64 _frameId) { KE_ERROR("NYI"); return {}; }

        bool DestroyTextureSrv(TextureSrvHandle _handle) { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] SamplerHandle CreateSampler(const SamplerDesc& _samplerDesc) { KE_ERROR("NYI"); return {}; }
        bool DestroySampler(SamplerHandle _sampler) { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] RenderTargetViewHandle CreateRenderTargetView(const RenderTargetViewDesc& _desc);
        bool DestroyRenderTargetView(RenderTargetViewHandle _handle);

        [[nodiscard]] RenderTargetViewHandle GetPresentRenderTargetView(u8 _swapChainIndex) { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] TextureHandle GetPresentTexture(u8 _swapChainIndex) { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] u32 GetCurrentPresentImageIndex() const { KE_ERROR("NYI"); return {}; }

        [[nodiscard]] RenderPassHandle CreateRenderPass(const RenderPassDesc& _desc) { KE_ERROR("NYI"); return {}; }

        bool DestroyRenderPass(RenderPassHandle _handle) { KE_ERROR("NYI"); return {}; }

        CommandList BeginGraphicsCommandList(u64 _frameId);
        void EndGraphicsCommandList(u64 _frameId);

        void BeginRenderPass(CommandList _commandList, RenderPassHandle _handle) { KE_ERROR("NYI"); return; }

        void EndRenderPass(CommandList _commandList) { KE_ERROR("NYI"); return; }

        void SetTextureData(
            CommandList _commandList,
            BufferHandle _stagingBuffer,
            TextureHandle _dstTexture,
            const TextureMemoryFootprint& _footprint,
            const SubResourceIndexing& _subResourceIndex,
            void* _data) { KE_ERROR("NYI"); return; }

        void MapBuffer(BufferMapping& _mapping) { KE_ERROR("NYI"); return; }

        void UnmapBuffer(BufferMapping& _mapping) { KE_ERROR("NYI"); return; }

        void CopyBuffer(CommandList _commandList, const BufferCopyParameters& _params) { KE_ERROR("NYI"); return; }

        void PlaceMemoryBarriers(
            CommandList _commandList,
            const eastl::span<GlobalMemoryBarrier>& _globalMemoryBarriers,
            const eastl::span<BufferMemoryBarrier>& _bufferMemoryBarriers,
            const eastl::span<TextureMemoryBarrier>& _textureMemoryBarriers) { KE_ERROR("NYI"); return; }

        [[nodiscard]] ShaderModuleHandle RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize) { KE_ERROR("NYI"); return {}; }
        [[nodiscard]] DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetDesc& _desc, u32* _bindingIndices) { KE_ERROR("NYI"); return {}; }
        [[nodiscard]] DescriptorSetHandle CreateDescriptorSet(DescriptorSetLayoutHandle _layout) { KE_ERROR("NYI"); return {}; }
        [[nodiscard]] PipelineLayoutHandle CreatePipelineLayout(const PipelineLayoutDesc& _desc) { KE_ERROR("NYI"); return {}; }
        [[nodiscard]] GraphicsPipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc){ KE_ERROR("NYI"); return {}; }
        bool DestroyGraphicsPipeline(GraphicsPipelineHandle _pipeline) { KE_ERROR("NYI"); return {}; }
        bool DestroyPipelineLayout(PipelineLayoutHandle _layout) { KE_ERROR("NYI"); return {}; }
        bool DestroyDescriptorSet(DescriptorSetHandle _set) { KE_ERROR("NYI"); return {}; }
        bool DestroyDescriptorSetLayout(DescriptorSetLayoutHandle _layout) { KE_ERROR("NYI"); return {}; }
        bool FreeShaderModule(ShaderModuleHandle _module){ KE_ERROR("NYI"); return {}; }

        void UpdateDescriptorSet(
            DescriptorSetHandle _descriptorSet,
            const eastl::span<DescriptorSetWriteInfo>& _writes,
            u64 _frameId) { KE_ERROR("NYI"); return; }

        void SetViewport(CommandList _commandList, const Viewport& _viewport){ KE_ERROR("NYI"); return; }
        void SetScissorsRect(CommandList _commandList, const Rect& _rect) { KE_ERROR("NYI"); return; }
        void SetIndexBuffer(CommandList _commandList, const BufferView& _indexBufferView, bool _isU16) { KE_ERROR("NYI"); return; }
        void SetVertexBuffers(CommandList _commandList, const eastl::span<BufferView>& _bufferViews) { KE_ERROR("NYI"); return; }
        void SetGraphicsPipeline(CommandList _commandList, GraphicsPipelineHandle _graphicsPipeline) { KE_ERROR("NYI"); return; }
        void SetGraphicsPushConstant(
            CommandList _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<u32>& _data,
            u32 _index,
            u32 _offset) { KE_ERROR("NYI"); return; }
        void SetGraphicsDescriptorSets(
            CommandList _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<DescriptorSetHandle>& _sets,
            const bool* _unchanged,
            u64 _frameId) { KE_ERROR("NYI"); return; }
        void DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc) { KE_ERROR("NYI"); return; }

    private:
        MetalResources m_resources;
    };
} // KryneEngine
