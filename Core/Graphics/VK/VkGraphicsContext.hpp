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
#include <Graphics/Common/Texture.hpp>
#include <Graphics/VK/CommonStructures.hpp>

namespace KryneEngine
{
    class Window;
    class VkSurface;
    class VkSwapChain;
    struct RenderTargetViewDesc;
    class VkDebugHandler;

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

        [[nodiscard]] GenPool::Handle CreateTextureSrv(const TextureSrvDesc& _srvDesc, u64 /*_frameId*/)
        {
            return m_resources.CreateTextureSrv(_srvDesc, m_device);
        }

        [[nodiscard]] inline GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc)
        {
            return m_resources.CreateRenderTargetView(_desc, m_device);
        }

        bool DestroyRenderTargetView(GenPool::Handle _handle)
        {
            return m_resources.FreeRenderTargetView(_handle, m_device);
        }

        GenPool::Handle GetPresentRenderTarget(u8 _index);
        [[nodiscard]] u32 GetCurrentPresentImageIndex() const;

        [[nodiscard]] GenPool::Handle CreateRenderPass(const RenderPassDesc& _desc)
        {
            return m_resources.CreateRenderPass(_desc, m_device);
        }

        bool DestroyRenderPass(GenPool::Handle _handle)
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

        void BeginRenderPass(CommandList _commandList, GenPool::Handle _handle);
        void EndRenderPass(CommandList _commandList);

    private:
        VkResources m_resources {};
    };
}


