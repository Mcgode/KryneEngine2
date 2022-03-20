/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "VkGraphicsContext.hpp"

#include <iostream>
#include <EASTL/algorithm.h>
#include <EASTL/vector_map.h>
#include <Graphics/VK/HelperFunctions.hpp>
#include <GLFW/glfw3.h>

namespace KryneEngine
{
    using namespace VkHelperFunctions;

    namespace
    {
        static const eastl::vector<const char*> kValidationLayerNames = {
                "VK_LAYER_KHRONOS_validation"
        };

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT _messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* _pCallbackData,
                void* _pUserData)
        {
            eastl::string severity = "|";
            if (_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            {
                severity += "verbose|";
            }
            if (_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            {
                severity += "info|";
            }
            if (_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                severity += "warning|";
            }
            if (_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                severity += "error|";
            }

            if (_messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                std::cerr << "Validation layer (" << severity.c_str() << "): " << _pCallbackData->pMessage << std::endl;
            }

            return VK_FALSE;
        }
    }

    VkGraphicsContext::VkGraphicsContext(const GraphicsCommon::ApplicationInfo& _appInfo)
        : m_appInfo(_appInfo)
    {
        vk::ApplicationInfo applicationInfo(
                m_appInfo.m_applicationName,
                MakeVersion(m_appInfo.m_applicationVersion),
                "KryneEngine2",
                MakeVersion(m_appInfo.m_engineVersion),
                GetApiVersion(m_appInfo.m_api));

        vk::InstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledLayerCount = 0;

        const auto& availableExtensions = vk::enumerateInstanceExtensionProperties();
        std::cout << "Available extensions:" << std::endl;
        for (const auto& extension: availableExtensions)
        {
            std::cout << "\t" << extension.extensionName << std::endl;
        }

        vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
        if (m_appInfo.m_features.m_validationLayers)
        {
            _PrepareValidationLayers(instanceCreateInfo);
            debugMessengerCreateInfo = _PopulateDebugCreateInfo(this);
            instanceCreateInfo.pNext = &debugMessengerCreateInfo;
        }

        auto extensions = _RetrieveRequiredExtensionNames(m_appInfo);
        instanceCreateInfo.enabledExtensionCount = extensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

        VkAssert(vk::createInstance(&instanceCreateInfo, nullptr, &m_instance));

        _SetupValidationLayersCallback();
        _SelectPhysicalDevice();
        _CreateDevice();
    }

    VkGraphicsContext::~VkGraphicsContext()
    {
        if (m_debugMessenger)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) m_instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(m_instance, m_debugMessenger, nullptr);
            }
        }
        m_instance.destroy();
    }

    void VkGraphicsContext::_PrepareValidationLayers(vk::InstanceCreateInfo& _createInfo)
    {
        const auto &availableLayers = vk::enumerateInstanceLayerProperties();

        bool found = false;
        for (const auto &validationLayerName: kValidationLayerNames)
        {
            for (const auto &layerProperty: availableLayers)
            {
                if (strcmp(validationLayerName, layerProperty.layerName) == 0)
                {
                    found = true;
                }
            }

            if (found)
            {
                break;
            }
        }

        if (Verify(found))
        {
            _createInfo.ppEnabledLayerNames = kValidationLayerNames.data();
            _createInfo.enabledLayerCount = kValidationLayerNames.size();
        }
    }

    eastl::vector<const char *> VkGraphicsContext::_RetrieveRequiredExtensionNames(const GraphicsCommon::ApplicationInfo& _appInfo)
    {
        u32 glfwCount;
        const char** ppGlfwExtensions = glfwGetRequiredInstanceExtensions(&glfwCount);

        eastl::vector<const char *> result(ppGlfwExtensions, ppGlfwExtensions + glfwCount);

        if (_appInfo.m_features.m_validationLayers)
        {
            result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return result;
    }

    vk::DebugUtilsMessengerCreateInfoEXT VkGraphicsContext::_PopulateDebugCreateInfo(void *_userData)
    {
        const auto severityFlags =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        const auto messageTypeFlags =
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

        return {{}, severityFlags, messageTypeFlags,DebugCallback, _userData};
    }

    void VkGraphicsContext::_SetupValidationLayersCallback()
    {
        auto createInfo = _PopulateDebugCreateInfo(this);
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)m_instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");

        if (Verify(func != nullptr))
        {
            VkAssert(func(m_instance, reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo),
                          nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT*>(&m_debugMessenger)));
        }
    }

    void VkGraphicsContext::_SelectPhysicalDevice()
    {
        const auto availableDevices = m_instance.enumeratePhysicalDevices();
        eastl::vector<vk::PhysicalDevice> suitableDevices;
        eastl::copy_if(availableDevices.begin(), availableDevices.end(),
                       eastl::back_inserter(suitableDevices),
                       [this](const vk::PhysicalDevice& _physicalDevice)
        {
            const auto properties = _physicalDevice.getProperties();
            const auto features = _physicalDevice.getFeatures();

            bool suitable = true;

            auto placeholderQueueIndices = QueueIndices();
            suitable &= _SelectQueues(m_appInfo, _physicalDevice, placeholderQueueIndices);

//            if (_appInfo.m_features.m_display)
//            {
//                suitable &= features.geometryShader;
//            }

            return suitable;
        });

        if (Verify(!suitableDevices.empty(), "No suitable device found!"))
        {
            u32 maxScore = 0;
            vk::PhysicalDevice selectedDevice;

            for (const auto& suitableDevice: suitableDevices)
            {
                u32 score = 0;
                const auto properties = suitableDevice.getProperties();
                score += properties.limits.maxImageDimension2D;

                if (score >= maxScore)
                {
                    selectedDevice = suitableDevice;
                }
            }

            m_physicalDevice = selectedDevice;
        }
    }

    bool VkGraphicsContext::_SelectQueues(const GraphicsCommon::ApplicationInfo &_appInfo,
                                          const vk::PhysicalDevice &_device,
                                          QueueIndices &_indices)
    {
        const auto familyProperties = _device.getQueueFamilyProperties();

        bool foundAll = true;

        const auto& features = _appInfo.m_features;

        Assert(features.m_transfer && (features.m_graphics || features.m_transferQueue), "Not supported yet");
        Assert(features.m_compute && (features.m_graphics || features.m_asyncCompute), "Not supported yet");

        if (features.m_graphics)
        {
            for (s8 i = 0; i < familyProperties.size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;

                const bool graphicsOk = bool(flags & vk::QueueFlagBits::eGraphics);
                const bool transferOk = !features.m_transfer || features.m_transferQueue || bool(flags & vk::QueueFlagBits::eTransfer);
                const bool computeOk = !features.m_compute || features.m_asyncCompute || bool(flags & vk::QueueFlagBits::eCompute);

                if (graphicsOk && transferOk && computeOk)
                {
                    _indices.m_graphicsQueueIndex = i;
                    break;
                }
            }
            foundAll &= _indices.m_graphicsQueueIndex != QueueIndices::kInvalid;
        }

        if (features.m_transferQueue)
        {
            u8 topScore = 0;
            s8 topIndex = QueueIndices::kInvalid;
            for (s8 i = 0; i < familyProperties.size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;
                if (flags & vk::QueueFlagBits::eTransfer)
                {
                    u8 score = 0;
                    score += flags & vk::QueueFlagBits::eGraphics ? 0 : 4;
                    score += flags & vk::QueueFlagBits::eCompute ? 0 : 3;

                    if (score > topScore)
                    {
                        topScore = score;
                        topIndex = i;
                    }
                }
            }
            _indices.m_transferQueueIndex = topIndex;
            foundAll &= _indices.m_transferQueueIndex != QueueIndices::kInvalid;
        }

        if (features.m_asyncCompute)
        {
            u8 topScore = 0;
            s8 topIndex = QueueIndices::kInvalid;
            for (s8 i = 0; i < familyProperties.size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;
                if (flags & vk::QueueFlagBits::eCompute)
                {
                    u8 score = 0;
                    score += flags & vk::QueueFlagBits::eTransfer ? 0 : 1;
                    score += flags & vk::QueueFlagBits::eGraphics ? 0 : 3;

                    if (score > topScore)
                    {
                        topScore = score;
                        topIndex = i;
                    }
                }
            }
            _indices.m_computeQueueIndex = topIndex;
            foundAll &= _indices.m_computeQueueIndex != QueueIndices::kInvalid;
        }

        return foundAll;
    }

    void VkGraphicsContext::_CreateDevice()
    {
        eastl::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
        eastl::vector<float> queuePriorities;

        QueueIndices queueIndices;
        Assert(_SelectQueues(m_appInfo, m_physicalDevice, queueIndices));
        {
            const auto createQueueInfo = [&queueCreateInfo, &queuePriorities](s8 _index, float _priority)
            {
                if (_index != QueueIndices::kInvalid)
                {
                    auto& createInfo = queueCreateInfo.emplace_back();
                    createInfo.queueFamilyIndex = static_cast<u32>(_index);
                    createInfo.queueCount = 1;
                    auto& priority = queuePriorities.emplace_back();
                    priority = _priority;
                    createInfo.pQueuePriorities = &priority;
                }
            };

            createQueueInfo(queueIndices.m_graphicsQueueIndex, 1.0);
            createQueueInfo(queueIndices.m_transferQueueIndex, 0.5);
            createQueueInfo(queueIndices.m_computeQueueIndex, 0.5);
        }

        vk::PhysicalDeviceFeatures features;

        vk::ArrayProxyNoTemporaries<const char* const> enabledLayerNames;
        if (m_appInfo.m_features.m_validationLayers)
        {
            enabledLayerNames = MakeArrayProxy(kValidationLayerNames);
        }

        vk::DeviceCreateInfo createInfo({},MakeArrayProxy(queueCreateInfo),
                                        enabledLayerNames,
                                        {}, &features);

        VkAssert(m_physicalDevice.createDevice(&createInfo, nullptr, &m_device));

        _RetrieveQueues(queueIndices);
    }

    void VkGraphicsContext::_RetrieveQueues(const QueueIndices &_queueIndices)
    {
        eastl::vector_map<u32, u32> queueIndexPerFamily;

        const auto RetrieveQueue = [this, &queueIndexPerFamily](s8 _queueIndex, vk::Queue& destQueue_)
        {
            if (_queueIndex != QueueIndices::kInvalid)
            {
                const u32 familyIndex = _queueIndex;
                auto it = queueIndexPerFamily.find(familyIndex);
                if (it == queueIndexPerFamily.end())
                {
                    it = queueIndexPerFamily.emplace(familyIndex, 0).first;
                }

                destQueue_ = m_device.getQueue(familyIndex, it->second++);
            }
        };

        RetrieveQueue(_queueIndices.m_graphicsQueueIndex, m_graphicsQueue);
        RetrieveQueue(_queueIndices.m_transferQueueIndex, m_transferQueue);
        RetrieveQueue(_queueIndices.m_computeQueueIndex, m_computeQueue);
    }
}
