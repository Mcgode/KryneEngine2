/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include "Handles.hpp"
#include "KryneEngine/Core/Graphics/ResourceViews/BufferView.hpp"
#include "KryneEngine/Core/Graphics/ResourceViews/TextureView.hpp"
#include "Texture.hpp"

namespace KryneEngine
{
    namespace GraphicsCommon
    {
        struct ApplicationInfo;
    }

    struct BufferCopyParameters;
    struct BufferCreateDesc;
    struct BufferMapping;
    struct BufferMemoryBarrier;
    struct BufferSpan;
    struct ComputePipelineDesc;
    struct DescriptorSetDesc;
    struct DescriptorSetWriteInfo;
    struct DrawIndexedInstancedDesc;
    struct DrawInstancedDesc;
    struct GlobalMemoryBarrier;
    struct GraphicsPipelineDesc;
    struct PipelineLayoutDesc;
    struct RenderTargetViewDesc;
    struct RenderPassDesc;
    struct TextureViewDesc;
    struct TextureMemoryBarrier;
    struct Viewport;

    class Window;

    using CommandListHandle = void*;

    class GraphicsContext
    {
    public:
        static GraphicsContext* Create(
            const GraphicsCommon::ApplicationInfo& _appInfo,
            Window* _window,
            AllocatorInstance _allocator);
        static void Destroy(GraphicsContext* _context);

        [[nodiscard]] inline u64 GetFrameId() const
        {
            return m_frameId;
        }
        [[nodiscard]] u8 GetFrameContextCount() const;
        [[nodiscard]] inline u8 GetCurrentFrameContextIndex() const
        {
            return m_frameId % GetFrameContextCount();
        }

        bool EndFrame();
        void WaitForLastFrame() const;
        [[nodiscard]] bool IsFrameExecuted(u64 _frameId) const;

        [[nodiscard]] const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const;
        [[nodiscard]] static const char* GetShaderFileExtension();

        [[nodiscard]] bool HasDedicatedTransferQueue() const;
        [[nodiscard]] bool HasDedicatedComputeQueue() const;

    private:

        AllocatorInstance m_allocator;
        const Window* m_window;

        static constexpr u64 kInitialFrameId = 1;
        u64 m_frameId;

    public:

        [[nodiscard]] BufferHandle CreateBuffer(const BufferCreateDesc& _desc);
        [[nodiscard]] bool NeedsStagingBuffer(BufferHandle _buffer);
        bool DestroyBuffer(BufferHandle _bufferHandle);

        [[nodiscard]] TextureHandle CreateTexture(const TextureCreateDesc& _createDesc);
        [[nodiscard]] eastl::vector<TextureMemoryFootprint> FetchTextureSubResourcesMemoryFootprints(const TextureDesc& _desc);
        [[nodiscard]] BufferHandle CreateStagingBuffer(
            const TextureDesc& _createDesc,
            const eastl::span<const TextureMemoryFootprint>& _footprints);
        bool DestroyTexture(TextureHandle _handle);

        [[nodiscard]] TextureViewHandle CreateTextureView(const TextureViewDesc& _viewDesc);
        bool DestroyTextureView(TextureViewHandle _handle);

        [[nodiscard]] SamplerHandle CreateSampler(const SamplerDesc& _samplerDesc);
        bool DestroySampler(SamplerHandle _sampler);

        [[nodiscard]] BufferViewHandle CreateBufferView(const BufferViewDesc& _viewDesc);
        bool DestroyBufferView(BufferViewHandle _handle);

        [[nodiscard]] RenderTargetViewHandle CreateRenderTargetView(const RenderTargetViewDesc& _desc);
        bool DestroyRenderTargetView(RenderTargetViewHandle _handle);

        [[nodiscard]] RenderTargetViewHandle GetPresentRenderTargetView(u8 _swapChainIndex);
        [[nodiscard]] TextureHandle GetPresentTexture(u8 _swapChainIndex);
        [[nodiscard]] u32 GetCurrentPresentImageIndex() const;

        [[nodiscard]] RenderPassHandle CreateRenderPass(const RenderPassDesc& _desc);
        bool DestroyRenderPass(RenderPassHandle _handle);

        CommandListHandle BeginGraphicsCommandList();
        void EndGraphicsCommandList(CommandListHandle _commandList);

        void BeginRenderPass(CommandListHandle _commandList, RenderPassHandle _handle);
        void EndRenderPass(CommandListHandle _commandList);

        void BeginComputePass(CommandListHandle _commandList);
        void EndComputePass(CommandListHandle _commandList);

        void SetTextureData(
            CommandListHandle _commandList,
            BufferHandle _stagingBuffer,
            TextureHandle _dstTexture,
            const TextureMemoryFootprint& _footprint,
            const SubResourceIndexing& _subResourceIndex,
            const void* _data);

        void MapBuffer(BufferMapping& _mapping);
        void UnmapBuffer(BufferMapping& _mapping);

        void CopyBuffer(CommandListHandle _commandList, const BufferCopyParameters& _params);

        [[nodiscard]] static bool SupportsNonGlobalBarriers();
        void PlaceMemoryBarriers(
            CommandListHandle _commandList,
            const eastl::span<const GlobalMemoryBarrier>& _globalMemoryBarriers,
            const eastl::span<const BufferMemoryBarrier>& _bufferMemoryBarriers,
            const eastl::span<const TextureMemoryBarrier>& _textureMemoryBarriers);

        [[nodiscard]] static bool RenderPassNeedsUsageDeclaration();
        [[nodiscard]] static bool ComputePassNeedsUsageDeclaration();
        void DeclarePassTextureViewUsage(
            CommandListHandle _commandList,
            const eastl::span<const TextureViewHandle>& _textures,
            KryneEngine::TextureViewAccessType _accessType);
        void DeclarePassBufferViewUsage(
            CommandListHandle _commandList,
            const eastl::span<const BufferViewHandle>& _buffers,
            BufferViewAccessType _accessType);;

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

        [[nodiscard]] ComputePipelineHandle CreateComputePipeline(const ComputePipelineDesc& _desc);
        bool DestroyComputePipeline(ComputePipelineHandle _pipeline);

        void UpdateDescriptorSet(
            DescriptorSetHandle _descriptorSet,
            const eastl::span<const DescriptorSetWriteInfo>& _writes);

        void SetViewport(CommandListHandle _commandList, const Viewport& _viewport);
        void SetScissorsRect(CommandListHandle _commandList, const Rect& _rect);
        void SetIndexBuffer(CommandListHandle _commandList, const BufferSpan& _indexBufferView, bool _isU16 = false);
        void SetVertexBuffers(CommandListHandle _commandList, const eastl::span<const BufferSpan>& _bufferViews);
        void SetGraphicsPipeline(CommandListHandle _commandList, GraphicsPipelineHandle _graphicsPipeline);
        void SetGraphicsPushConstant(
            CommandListHandle _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<const u32>& _data,
            u32 _index = 0,
            u32 _offset = 0);
        void SetGraphicsDescriptorSets(
            CommandListHandle _commandList,
            PipelineLayoutHandle _layout,
            const eastl::span<const DescriptorSetHandle>& _sets,
            const bool* _unchanged = nullptr);

        void DrawInstanced(CommandListHandle _commandList, const DrawInstancedDesc& _desc);
        void DrawIndexedInstanced(CommandListHandle _commandList, const DrawIndexedInstancedDesc& _desc);

        void SetComputePipeline(CommandListHandle _commandList, ComputePipelineHandle _pipeline);
        void SetComputeDescriptorSets(
            CommandListHandle _commandList,
            PipelineLayoutHandle _layout,
            eastl::span<const DescriptorSetHandle> _sets,
            u32 _offset = 0);
        void SetComputePushConstant(
            CommandListHandle _commandList,
            PipelineLayoutHandle _layout,
            eastl::span<const u32> _data);

        void Dispatch(CommandListHandle _commandList, uint3 _threadGroupCount, uint3 _threadGroupSize);
    };
}


