/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include "VkTypes.hpp"
#include "VkFrameContext.hpp"
#include "VkResources.hpp"
#include <EASTL/unique_ptr.h>
#include <EASTL/vector_set.h>
#include <Graphics/Common/MemoryBarriers.hpp>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/VK/CommonStructures.hpp>

namespace KryneEngine
{
    class VkDebugHandler;
    class VkDescriptorSetManager;
    class VkSurface;
    class VkSwapChain;
    class Window;

    struct BufferMapping;
    struct BufferCopyParameters;
    struct BufferView;
    struct DrawIndexedInstancedDesc;
    struct Viewport;

    class VkGraphicsContext
    {
    public:
        explicit VkGraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo, u64 _frameId);

        virtual ~VkGraphicsContext();

        [[nodiscard]] Window* GetWindow() const { return m_window.get(); }

        [[nodiscard]] u8 GetFrameContextCount() const { return m_frameContextCount; }

        void EndFrame(u64 _frameId);

        [[nodiscard]] bool IsFrameExecuted(u64 _frameId) const;

        void WaitForFrame(u64 _frameId) const;

        [[nodiscard]] const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const { return m_appInfo; }

    private:
        const GraphicsCommon::ApplicationInfo m_appInfo;

        eastl::unique_ptr<Window> m_window;

        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;

        eastl::unique_ptr<VkSurface> m_surface;
        eastl::unique_ptr<VkSwapChain> m_swapChain;

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

        static bool _SelectQueues(const GraphicsCommon::ApplicationInfo &_appInfo, const VkPhysicalDevice &_physicalDevice,
                                  const VkSurfaceKHR &_surface, VkCommonStructures::QueueIndices &_indices);

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
            const eastl::vector<TextureMemoryFootprint>& _footprints)
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

        [[nodiscard]] TextureSrvHandle CreateTextureSrv(const TextureSrvDesc& _srvDesc, u64 /*_frameId*/)
        {
            return m_resources.CreateTextureSrv(_srvDesc, m_device);
        }

        inline bool DestroyTextureSrv(TextureSrvHandle _handle)
        {
            return m_resources.DestroyTextureSrv(_handle, m_device);
        }

        [[nodiscard]] SamplerHandle CreateSampler(const SamplerDesc& _samplerDesc);
        bool DestroySampler(SamplerHandle _sampler);

        [[nodiscard]] inline RenderTargetViewHandle CreateRenderTargetView(const RenderTargetViewDesc& _desc)
        {
            return m_resources.CreateRenderTargetView(_desc, m_device);
        }

        bool DestroyRenderTargetView(RenderTargetViewHandle _handle)
        {
            return m_resources.FreeRenderTargetView(_handle, m_device);
        }

        RenderTargetViewHandle GetPresentRenderTargetView(u8 _index);
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

        void EndGraphicsCommandList(u64 _frameId)
        {
            m_frameContexts[_frameId % m_frameContextCount].EndGraphicsCommandBuffer();
        }

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
        [[nodiscard]] DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetDesc& _desc, u32* _bindingIndices);
        [[nodiscard]] DescriptorSetHandle CreateDescriptorSet(DescriptorSetLayoutHandle _layout);
        [[nodiscard]] PipelineLayoutHandle CreatePipelineLayout(const PipelineLayoutDesc& _desc);
        [[nodiscard]] GraphicsPipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc);

        void UpdateDescriptorSet(
            DescriptorSetHandle _descriptorSet,
            const eastl::span<DescriptorSetWriteInfo>& _writes,
            u64 _frameId);

        void SetViewport(CommandList  _commandList, const Viewport& _viewport);
        void SetScissorsRect(CommandList  _commandList, const Rect& _rect);
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
            u32 _frameId);
        void DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc);

    private:
        VkResources m_resources {};
        eastl::unique_ptr<VkDescriptorSetManager> m_descriptorSetManager;
    };
}


