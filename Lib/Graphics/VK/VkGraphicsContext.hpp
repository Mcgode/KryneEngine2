/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/vector_set.h>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Graphics/VK/CommonStructures.hpp>

namespace KryneEngine
{
    class Window;
    class VkSurface;

    class VkGraphicsContext
    {
    public:
        explicit VkGraphicsContext(const GraphicsCommon::ApplicationInfo& _appInfo);

        virtual ~VkGraphicsContext();

        [[nodiscard]] Window* GetWindow() const { return m_window.get(); }

    private:
        const GraphicsCommon::ApplicationInfo m_appInfo;

        eastl::unique_ptr<Window> m_window;

        VkSharedInstance m_sharedInstance;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::PhysicalDevice m_physicalDevice;
        VkSharedDevice m_sharedDevice;

        eastl::unique_ptr<VkSurface> m_surface;

        vk::Queue m_graphicsQueue;
        vk::Queue m_transferQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_presentQueue;

        static void _PrepareValidationLayers(vk::InstanceCreateInfo& _createInfo);

        static eastl::vector<const char*> _RetrieveRequiredExtensionNames(const GraphicsCommon::ApplicationInfo& _appInfo);

        static vk::DebugUtilsMessengerCreateInfoEXT _PopulateDebugCreateInfo(void *_userData);

        void _SetupValidationLayersCallback();

        [[nodiscard]] eastl::vector_set<eastl::string> _GetRequiredDeviceExtensions() const;
        void _SelectPhysicalDevice();

        static bool _SelectQueues(const GraphicsCommon::ApplicationInfo &_appInfo, const vk::PhysicalDevice &_device,
                                  const vk::SurfaceKHR &_surface, VkCommonStructures::QueueIndices &_indices);

        void _CreateDevice();
        void _RetrieveQueues(const VkCommonStructures::QueueIndices &_queueIndices);
    };
}


