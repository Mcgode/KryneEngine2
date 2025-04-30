/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include "Handles.hpp"
#include "Texture.hpp"

namespace KryneEngine
{
    namespace GraphicsCommon
    {
        struct ApplicationInfo;
    }

    struct BufferCbvDesc;
    struct BufferCopyParameters;
    struct BufferCreateDesc;
    struct BufferMapping;
    struct BufferMemoryBarrier;
    struct BufferView;
    struct DescriptorSetDesc;
    struct DescriptorSetWriteInfo;
    struct DrawIndexedInstancedDesc;
    struct DrawInstancedDesc;
    struct GlobalMemoryBarrier;
    struct GraphicsPipelineDesc;
    struct PipelineLayoutDesc;
    struct RenderTargetViewDesc;
    struct RenderPassDesc;
    struct TextureSrvDesc;
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

        [[nodiscard]] TextureSrvHandle CreateTextureSrv(const TextureSrvDesc& _srvDesc);
        bool DestroyTextureSrv(TextureSrvHandle _handle);

        [[nodiscard]] SamplerHandle CreateSampler(const SamplerDesc& _samplerDesc);
        bool DestroySampler(SamplerHandle _sampler);

        [[nodiscard]] BufferCbvHandle CreateBufferCbv(const BufferCbvDesc& _cbvDesc);
        bool DestroyBufferCbv(BufferCbvHandle _handle);

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

        void SetTextureData(
            CommandListHandle _commandList,
            BufferHandle _stagingBuffer,
            TextureHandle _dstTexture,
            const TextureMemoryFootprint& _footprint,
            const SubResourceIndexing& _subResourceIndex,
            void* _data);

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
        void DeclarePassTextureSrvUsage(CommandListHandle _commandList, const eastl::span<const TextureSrvHandle>& _textures);
        void DeclarePassBufferCbvUsage(CommandListHandle _commandList, const eastl::span<const BufferCbvHandle>& _buffers);

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
            const eastl::span<const DescriptorSetWriteInfo>& _writes);

        void SetViewport(CommandListHandle _commandList, const Viewport& _viewport);
        void SetScissorsRect(CommandListHandle _commandList, const Rect& _rect);
        void SetIndexBuffer(CommandListHandle _commandList, const BufferView& _indexBufferView, bool _isU16 = false);
        void SetVertexBuffers(CommandListHandle _commandList, const eastl::span<const BufferView>& _bufferViews);
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
    };
}


