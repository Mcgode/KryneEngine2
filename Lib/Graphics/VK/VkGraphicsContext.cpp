/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "VkGraphicsContext.hpp"

#include <iostream>
#include <EASTL/algorithm.h>
#include <EASTL/vector_map.h>
#include <Common/KEStringHelpers.hpp>
#include <Graphics/Common/Window.hpp>
#include <Graphics/VK/HelperFunctions.hpp>
#include <Graphics/VK/VkSurface.hpp>
#include <GLFW/glfw3.h>

namespace KryneEngine
{
    using namespace VkHelperFunctions;
    using namespace VkCommonStructures;

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
        , m_sharedInstance({})
        , m_sharedDevice({})
    {
        if (m_appInfo.m_features.m_present)
        {
            m_window = eastl::make_unique<Window>(Window::Params());
        }

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

        VkAssert(vk::createInstance(&instanceCreateInfo, nullptr, &m_sharedInstance.m_object));

        _SetupValidationLayersCallback();

        if (m_appInfo.m_features.m_present)
        {
            m_surface = eastl::make_unique<VkSurface>(eastl::move(m_sharedInstance.MakeRef()), m_window->GetGlfwWindow());
        }

        _SelectPhysicalDevice();

        if (m_appInfo.m_features.m_present)
        {
            m_surface->UpdateCapabilities(m_physicalDevice);
        }

        _CreateDevice();
    }

    VkGraphicsContext::~VkGraphicsContext()
    {
        m_surface.reset();
        m_sharedDevice.Destroy();
        if (m_debugMessenger)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) m_sharedInstance->getProcAddr("vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(*m_sharedInstance, m_debugMessenger, nullptr);
            }
        }
        m_sharedInstance.Destroy();
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
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)m_sharedInstance->getProcAddr("vkCreateDebugUtilsMessengerEXT");

        if (Verify(func != nullptr))
        {
            VkAssert(func(*m_sharedInstance, reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo),
                          nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT*>(&m_debugMessenger)));
        }
    }

    void VkGraphicsContext::_SelectPhysicalDevice()
    {
        const auto availableDevices = m_sharedInstance->enumeratePhysicalDevices();
        eastl::vector<vk::PhysicalDevice> suitableDevices;
        eastl::copy_if(availableDevices.begin(), availableDevices.end(),
                       eastl::back_inserter(suitableDevices),
                       [this](const vk::PhysicalDevice& _physicalDevice)
        {
            const auto properties = _physicalDevice.getProperties();
            const auto features = _physicalDevice.getFeatures();
            const auto extensions = _physicalDevice.enumerateDeviceExtensionProperties();
            auto requiredExtensions = _GetRequiredDeviceExtensions();

            bool suitable = true;

            auto placeholderQueueIndices = QueueIndices();
            suitable &= _SelectQueues(m_appInfo, _physicalDevice, m_surface->GetSurface(), placeholderQueueIndices);

            for (const auto& extension: extensions)
            {
                requiredExtensions.erase(eastl::string{ extension.extensionName });
            }
            suitable &= requiredExtensions.empty();

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

    bool
    VkGraphicsContext::_SelectQueues(const GraphicsCommon::ApplicationInfo &_appInfo,
                                     const vk::PhysicalDevice &_device,
                                     const vk::SurfaceKHR &_surface,
                                     QueueIndices &_indices)
    {
        const auto familyProperties = _device.getQueueFamilyProperties();
        eastl::vector_map<u32, u32> indices;

        bool foundAll = true;

        const auto& features = _appInfo.m_features;

        Assert(features.m_transfer && (features.m_graphics || features.m_transferQueue), "Not supported yet");
        Assert(features.m_compute && (features.m_graphics || features.m_asyncCompute), "Not supported yet");

        const auto GetIndexOfFamily = [&indices](u32 _familyIndex) -> u32&
        {
            auto it = indices.find(_familyIndex);
            if (it == indices.end())
            {
                it = indices.emplace(_familyIndex, 0).first;
            }
            return it->second;
        };

        if (features.m_graphics)
        {
            for (s8 i = 0; i < familyProperties.size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;

                const bool graphicsOk = bool(flags & vk::QueueFlagBits::eGraphics);
                const bool transferOk = !features.m_transfer || features.m_transferQueue || bool(flags & vk::QueueFlagBits::eTransfer);
                const bool computeOk = !features.m_compute || features.m_asyncCompute || bool(flags & vk::QueueFlagBits::eCompute);

                auto& index = GetIndexOfFamily(i);

                if (graphicsOk && transferOk && computeOk && index < familyProperties[i].queueCount)
                {
                    _indices.m_graphicsQueueIndex = { i, static_cast<s32>(index++) };
                    break;
                }
            }
            foundAll &= !_indices.m_graphicsQueueIndex.IsInvalid();
        }

        if (features.m_transferQueue)
        {
            u8 topScore = 0;
            s8 topIndex = QueueIndices::kInvalid;
            for (s8 i = 0; i < familyProperties.size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;
                if (flags & vk::QueueFlagBits::eTransfer && GetIndexOfFamily(i) < familyProperties[i].queueCount)
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
            _indices.m_transferQueueIndex = { topIndex,
                                              static_cast<s32>(GetIndexOfFamily(topIndex)++) };
            foundAll &= !_indices.m_transferQueueIndex.IsInvalid();
        }

        if (features.m_asyncCompute)
        {
            u8 topScore = 0;
            s8 topIndex = QueueIndices::kInvalid;
            for (s8 i = 0; i < familyProperties.size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;
                if (flags & vk::QueueFlagBits::eCompute && GetIndexOfFamily(i) < familyProperties[i].queueCount)
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
            _indices.m_computeQueueIndex = { topIndex,
                                             static_cast<s32>(GetIndexOfFamily(topIndex)++) };
            foundAll &= !_indices.m_computeQueueIndex.IsInvalid();
        }

        if (features.m_present)
        {
            u8 topScore = 0;
            s8 topIndex = QueueIndices::kInvalid;
            for (s8 i = 0; i < familyProperties.size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;
                if (_device.getSurfaceSupportKHR(i, _surface) && GetIndexOfFamily(i) < familyProperties[i].queueCount)
                {
                    u8 score = 0;
                    score += flags & vk::QueueFlagBits::eGraphics ? 1 : 5;
                    score += flags & vk::QueueFlagBits::eTransfer ? 1 : 4;
                    score += flags & vk::QueueFlagBits::eCompute ? 1 : 3;

                    if (score > topScore)
                    {
                        topScore = score;
                        topIndex = i;
                    }
                }
            }
            _indices.m_presentQueueIndex = {topIndex,
                                            static_cast<s32>(GetIndexOfFamily(topIndex)++) };
            foundAll &= !_indices.m_presentQueueIndex.IsInvalid();
        }

        return foundAll;
    }

    void VkGraphicsContext::_CreateDevice()
    {
        eastl::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
        eastl::vector<eastl::vector<float>> queuePriorities;

        QueueIndices queueIndices;
        Assert(_SelectQueues(m_appInfo, m_physicalDevice, m_surface->GetSurface(), queueIndices));
        {
            const auto createQueueInfo = [&queueCreateInfo, &queuePriorities](QueueIndices::Pair _index, float _priority)
            {
                if (_index.IsInvalid())
                {
                    return;
                }

                auto it = eastl::find(queueCreateInfo.begin(), queueCreateInfo.end(), _index.m_familyIndex,
                                      [](const vk::DeviceQueueCreateInfo& _info, u32 _familyIndex)
                                      { return _info.queueFamilyIndex == _familyIndex; });
                bool alreadyInserted = it != queueCreateInfo.end();

                if (!alreadyInserted)
                {
                    it = queueCreateInfo.emplace(queueCreateInfo.end());
                    queuePriorities.push_back();
                    it->queueFamilyIndex = _index.m_familyIndex;
                }
                it->queueCount++;
                auto& prioritiesVector = queuePriorities[eastl::distance(queueCreateInfo.begin(), it)];
                if (_index.m_indexInFamily + 1 >= prioritiesVector.size())
                {
                    prioritiesVector.resize(_index.m_indexInFamily + 1);
                    it->pQueuePriorities = prioritiesVector.data();
                }
                prioritiesVector[_index.m_indexInFamily] = _priority;
            };

            createQueueInfo(queueIndices.m_graphicsQueueIndex, 1.0);
            createQueueInfo(queueIndices.m_transferQueueIndex, 0.5);
            createQueueInfo(queueIndices.m_computeQueueIndex, 0.5);
            createQueueInfo(queueIndices.m_presentQueueIndex, 1.0);

            for (u32 i = 0; i < queueCreateInfo.size(); i++)
            {
                Assert(queueCreateInfo[i].queueCount == queuePriorities[i].size());
            }
        }

        vk::PhysicalDeviceFeatures features;

        const auto requiredExtensions = StringHelpers::RetrieveStringPointerContainer(_GetRequiredDeviceExtensions());

        vk::ArrayProxyNoTemporaries<const char* const> enabledLayerNames;
        if (m_appInfo.m_features.m_validationLayers)
        {
            enabledLayerNames = MakeArrayProxy(kValidationLayerNames);
        }

        vk::DeviceCreateInfo createInfo({}, MakeArrayProxy(queueCreateInfo),
                                        enabledLayerNames,
                                        MakeArrayProxy(requiredExtensions),
                                        &features);

        VkAssert(m_physicalDevice.createDevice(&createInfo, nullptr, &m_sharedDevice.m_object));

        _RetrieveQueues(queueIndices);
    }

    void VkGraphicsContext::_RetrieveQueues(const QueueIndices &_queueIndices)
    {
        const auto RetrieveQueue = [this](QueueIndices::Pair _queueIndex, vk::Queue& destQueue_)
        {
            if (!_queueIndex.IsInvalid())
            {
                destQueue_ = m_sharedDevice->getQueue(_queueIndex.m_familyIndex, _queueIndex.m_indexInFamily);
            }
        };

        RetrieveQueue(_queueIndices.m_graphicsQueueIndex, m_graphicsQueue);
        RetrieveQueue(_queueIndices.m_transferQueueIndex, m_transferQueue);
        RetrieveQueue(_queueIndices.m_computeQueueIndex, m_computeQueue);
        RetrieveQueue(_queueIndices.m_presentQueueIndex, m_presentQueue);
    }

    eastl::vector_set<eastl::string> VkGraphicsContext::_GetRequiredDeviceExtensions() const
    {
        eastl::vector_set<eastl::string> result;

        if (m_appInfo.m_features.m_present)
        {
            result.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }

        return result;
    }
}
