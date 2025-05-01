/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <EASTL/unique_ptr.h>
#include <EASTL/vector_set.h>

#include "Graphics/Vulkan/CommonStructures.hpp"
#include "Graphics/Vulkan/VkDescriptorSetManager.hpp"
#include "Graphics/Vulkan/VkFrameContext.hpp"
#include "Graphics/Vulkan/VkResources.hpp"
#include "Graphics/Vulkan/VkSurface.hpp"
#include "Graphics/Vulkan/VkSwapChain.hpp"
#include "Graphics/Vulkan/VkTypes.hpp"
#include "KryneEngine/Core/Graphics/MemoryBarriers.hpp"
#include "KryneEngine/Core/Graphics/ResourceViews/BufferView.hpp"
#include "KryneEngine/Core/Graphics/Texture.hpp"

namespace KryneEngine
{
    class VkDebugHandler;
    class Window;

    struct BufferMapping;
    struct BufferCopyParameters;
    struct BufferSpan;
    struct DrawIndexedInstancedDesc;
    struct DrawInstancedDesc;
    struct Viewport;

    class VkGraphicsContext
    {
    public:
        explicit VkGraphicsContext(
            AllocatorInstance _allocator,
            const GraphicsCommon::ApplicationInfo& _appInfo,
            Window* _window,
            u64 _frameId);

        virtual ~VkGraphicsContext();

        [[nodiscard]] u8 GetFrameContextCount() const { return m_frameContextCount; }

        void EndFrame(u64 _frameId);

        [[nodiscard]] bool IsFrameExecuted(u64 _frameId) const;

        void WaitForFrame(u64 _frameId) const;

        [[nodiscard]] const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const { return m_appInfo; }

        [[nodiscard]] bool HasDedicatedTransferQueue() const;
        [[nodiscard]] bool HasDedicatedComputeQueue() const;

    private:
        const GraphicsCommon::ApplicationInfo m_appInfo;
        AllocatorInstance m_allocator;

        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;

        VkSurface m_surface;
        VkSwapChain m_swapChain;

        VkCommonStructures::QueueIndices m_queueIndices {};
        VkQueue m_graphicsQueue;
        VkQueue m_transferQueue;
        VkQueue m_computeQueue;
        VkQueue m_presentQueue;

        bool m_debugUtils = false;
        bool m_debugMarkers = false;

        bool m_synchronization2 = false;
        PFN_vkCmdPipelineBarrier2KHR m_vkCmdPipelineBarrier2KHR = nullptr;

#if !defined(KE_FINAL)
        eastl::shared_ptr<VkDebugHandler> m_debugHandler;
#endif

        u8 m_frameContextCount;
        DynamicArray<VkFrameContext> m_frameContexts;

        static void _PrepareValidationLayers(VkInstanceCreateInfo& _createInfo);

        eastl::vector<const char*> _RetrieveRequiredExtensionNames(const GraphicsCommon::ApplicationInfo& _appInfo);
        void _RetrieveOptionalExtensionNames(
                eastl::vector<const char *>& _currentList,
                const DynamicArray<VkExtensionProperties> &_availableExtensions,
                const GraphicsCommon::ApplicationInfo &_appInfo);

        static VkDebugUtilsMessengerCreateInfoEXT _PopulateDebugCreateInfo(void *_userData);

        void _SetupValidationLayersCallback();

        [[nodiscard]] eastl::vector_set<eastl::string> _GetRequiredDeviceExtensions() const;
        void _SelectPhysicalDevice();

        bool _SelectQueues(
            const GraphicsCommon::ApplicationInfo &_appInfo,
            const VkPhysicalDevice &_physicalDevice,
            const VkSurfaceKHR &_surface,
            VkCommonStructures::QueueIndices &_indices);

        void _CreateDevice();
        void _RetrieveQueues(const VkCommonStructures::QueueIndices &_queueIndices);

    public:
        [[nodiscard]] eastl::vector<TextureMemoryFootprint> FetchTextureSubResourcesMemoryFootprints(
            const TextureDesc& _desc);

        [[nodiscard]] inline BufferHandle CreateBuffer(const BufferCreateDesc& _desc)
        {
            return m_resources.CreateBuffer(_desc, m_device);
        }

        [[nodiscard]] inline BufferHandle CreateStagingBuffer(
            const TextureDesc& _createDesc,
            const eastl::span<const TextureMemoryFootprint>& _footprints)
        {
            return m_resources.CreateStagingBuffer(_createDesc, _footprints, m_device);
        }

        [[nodiscard]] bool NeedsStagingBuffer(BufferHandle _buffer);

        inline bool DestroyBuffer(BufferHandle _bufferHandle)
        {
            return m_resources.DestroyBuffer(_bufferHandle);
        }

        [[nodiscard]] inline TextureHandle CreateTexture(const TextureCreateDesc& _createDesc)
        {
            return m_resources.CreateTexture(_createDesc, m_device);
        }

        inline bool DestroyTexture(TextureHandle _handle)
        {
            return m_resources.ReleaseTexture(_handle, m_device);
        }

        [[nodiscard]] TextureViewHandle CreateTextureView(const TextureViewDesc& _viewDesc, u64 /*_frameId*/)
        {
            return m_resources.CreateTextureView(_viewDesc, m_device);
        }

        inline bool DestroyTextureView(TextureViewHandle _handle)
        {
            return m_resources.DestroyTextureView(_handle, m_device);
        }

        [[nodiscard]] SamplerHandle CreateSampler(const SamplerDesc& _samplerDesc);
        bool DestroySampler(SamplerHandle _sampler);

        [[nodiscard]] BufferViewHandle CreateBufferView(const BufferViewDesc& _viewDesc);
        bool DestroyBufferView(BufferViewHandle _handle);

        [[nodiscard]] inline RenderTargetViewHandle CreateRenderTargetView(const RenderTargetViewDesc& _desc)
        {
            return m_resources.CreateRenderTargetView(_desc, m_device);
        }

        bool DestroyRenderTargetView(RenderTargetViewHandle _handle)
        {
            return m_resources.FreeRenderTargetView(_handle, m_device);
        }

        [[nodiscard]] RenderTargetViewHandle GetPresentRenderTargetView(u8 _index);
        [[nodiscard]] TextureHandle GetPresentTexture(u8 _swapChainIndex);
        [[nodiscard]] u32 GetCurrentPresentImageIndex() const;

        [[nodiscard]] RenderPassHandle CreateRenderPass(const RenderPassDesc& _desc)
        {
            return m_resources.CreateRenderPass(_desc, m_device);
        }

        bool DestroyRenderPass(RenderPassHandle _handle)
        {
            return m_resources.DestroyRenderPass(_handle, m_device);
        }

        CommandList BeginGraphicsCommandList(u64 _frameId)
        {
            return m_frameContexts[_frameId % m_frameContextCount].BeginGraphicsCommandBuffer(m_device);
        }

        void EndGraphicsCommandList(CommandList _commandList, u64 _frameId)
        {
            m_frameContexts[_frameId % m_frameContextCount].EndGraphicsCommandBuffer(_commandList);
        }

        void BeginRenderPass(CommandList _commandList, RenderPassHandle _renderPass);
        void EndRenderPass(CommandList _commandList);

        void BeginComputePass(CommandList _commandList) {};
        void EndComputePass(CommandList _commandList) {};

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

        [[nodiscard]] static bool SupportsNonGlobalBarriers() { return true; }
        void PlaceMemoryBarriers(
            CommandList _commandList,
            const eastl::span<const GlobalMemoryBarrier>& _globalMemoryBarriers,
            const eastl::span<const BufferMemoryBarrier>& _bufferMemoryBarriers,
            const eastl::span<const TextureMemoryBarrier>& _textureMemoryBarriers);

        [[nodiscard]] static bool RenderPassNeedsUsageDeclaration() { return false; }
        [[nodiscard]] static bool ComputePassNeedsUsageDeclaration() { return false; }
        void DeclarePassTextureViewUsage(CommandList, const eastl::span<const TextureViewHandle>&) {}
        void DeclarePassBufferViewUsage(CommandList, const eastl::span<const BufferViewHandle>&, BufferViewAccessType) {}

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
            const eastl::span<const DescriptorSetWriteInfo>& _writes,
            u64 _frameId);

        void SetViewport(CommandList  _commandList, const Viewport& _viewport);
        void SetScissorsRect(CommandList  _commandList, const Rect& _rect);
        void SetIndexBuffer(CommandList _commandList, const BufferSpan& _indexBufferView, bool _isU16);
        void SetVertexBuffers(CommandList _commandList, const eastl::span<const BufferSpan>& _bufferViews);
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
        void DrawInstanced(CommandList _commandList, const DrawInstancedDesc& _desc);
        void DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc);

    void SetComputePipeline(CommandList _commandList, ComputePipelineHandle _pipeline);
    void SetComputeDescriptorSets(
        CommandList _commandList,
        PipelineLayoutHandle _layout,
        eastl::span<const DescriptorSetHandle> _sets,
        u32 _offset,
        u64 _frameId);
    void SetComputePushConstant(
        CommandList _commandList,
        PipelineLayoutHandle _layout,
        eastl::span<const u32> _data);

    void Dispatch(CommandList _commandList, uint3 _threadGroupCount, uint3);

    private:
        VkResources m_resources;
        VkDescriptorSetManager m_descriptorSetManager;
    };
}


