/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include <Graphics/Common/GraphicsCommon.hpp>

namespace KryneEngine
{
    class Window;

    class VkGraphicsContext
    {
    public:
        explicit VkGraphicsContext(const GraphicsCommon::ApplicationInfo& _appInfo);

        virtual ~VkGraphicsContext();

        [[nodiscard]] Window* GetWindow() const { return m_window.get(); }

    private:
        struct QueueIndices
        {
            static constexpr s8 kInvalid = -1;
            s8 m_graphicsQueueIndex = kInvalid;
            s8 m_transferQueueIndex = kInvalid;
            s8 m_computeQueueIndex = kInvalid;
        };

        const GraphicsCommon::ApplicationInfo m_appInfo;

        eastl::unique_ptr<Window> m_window;

        vk::Instance m_instance;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::PhysicalDevice m_physicalDevice;
        vk::Device m_device;

        vk::Queue m_graphicsQueue;
        vk::Queue m_transferQueue;
        vk::Queue m_computeQueue;

        static void _PrepareValidationLayers(vk::InstanceCreateInfo& _createInfo);

        static eastl::vector<const char*> _RetrieveRequiredExtensionNames(const GraphicsCommon::ApplicationInfo& _appInfo);

        static vk::DebugUtilsMessengerCreateInfoEXT _PopulateDebugCreateInfo(void *_userData);

        void _SetupValidationLayersCallback();

        void _SelectPhysicalDevice();

        static bool _SelectQueues(const GraphicsCommon::ApplicationInfo &_appInfo, const vk::PhysicalDevice &_device,
                                  QueueIndices &_indices);

        void _CreateDevice();
        void _RetrieveQueues(const QueueIndices &_queueIndices);
    };
}


