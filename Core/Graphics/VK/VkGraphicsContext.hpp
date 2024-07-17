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
        explicit VkGraphicsContext(const GraphicsCommon::ApplicationInfo& _appInfo);

        virtual ~VkGraphicsContext();

        [[nodiscard]] Window* GetWindow() const { return m_window.get(); }

        [[nodiscard]] u8 GetFrameContextCount() const { return m_frameContextCount; }

        void EndFrame(u64 _frameId);

        void WaitForFrame(u64 _frameId) const;

        [[nodiscard]] const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const { return m_appInfo; }

    private:
        const GraphicsCommon::ApplicationInfo m_appInfo;

        eastl::unique_ptr<Window> m_window;

        vk::Instance m_instance;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::PhysicalDevice m_physicalDevice;
        vk::Device m_device;

        eastl::unique_ptr<VkSurface> m_surface;
        eastl::unique_ptr<VkSwapChain> m_swapChain;

        VkCommonStructures::QueueIndices m_queueIndices {};
        vk::Queue m_graphicsQueue;
        vk::Queue m_transferQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_presentQueue;

        bool m_debugUtils = false;
        bool m_debugMarkers = false;
#if !defined(KE_FINAL)
        eastl::shared_ptr<VkDebugHandler> m_debugHandler;
#endif

        u8 m_frameContextCount;
        DynamicArray<VkFrameContext> m_frameContexts;

        static void _PrepareValidationLayers(vk::InstanceCreateInfo& _createInfo);

        eastl::vector<const char*> _RetrieveRequiredExtensionNames(const GraphicsCommon::ApplicationInfo& _appInfo);
        void _RetrieveOptionalExtensionNames(
                eastl::vector<const char *>& _currentList,
                const std::vector<vk::ExtensionProperties> &_extensionsAvailable,
                const GraphicsCommon::ApplicationInfo &_appInfo);

        static vk::DebugUtilsMessengerCreateInfoEXT _PopulateDebugCreateInfo(void *_userData);

        void _SetupValidationLayersCallback();

        [[nodiscard]] eastl::vector_set<eastl::string> _GetRequiredDeviceExtensions() const;
        void _SelectPhysicalDevice();

        static bool _SelectQueues(const GraphicsCommon::ApplicationInfo &_appInfo, const vk::PhysicalDevice &_device,
                                  const vk::SurfaceKHR &_surface, VkCommonStructures::QueueIndices &_indices);

        void _CreateDevice();
        void _RetrieveQueues(const VkCommonStructures::QueueIndices &_queueIndices);

    public:
        [[nodiscard]] inline GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc)
        {
            return m_resources.CreateRenderTargetView(_desc, m_device);
        }

        bool DestroyRenderTargetView(GenPool::Handle _handle)
        {
            return m_resources.FreeRenderTargetView(_handle, m_device);
        }

        GenPool::Handle GetFrameContextPresentRenderTarget(u8 _index);

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


