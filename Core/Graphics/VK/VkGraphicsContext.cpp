/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "VkGraphicsContext.hpp"

#include <iostream>
#include <regex>
#include <EASTL/algorithm.h>
#include <EASTL/vector_map.h>
#include <Common/StringHelpers.hpp>
#include <Graphics/Common/Window.hpp>
#include <Graphics/VK/HelperFunctions.hpp>
#include <Graphics/VK/VkSurface.hpp>
#include <Graphics/VK/VkSwapChain.hpp>
#include <GLFW/glfw3.h>
#include "VkDebugHandler.hpp"

namespace KryneEngine
{
    using namespace VkHelperFunctions;
    using namespace VkCommonStructures;

    namespace
    {
        static const eastl::vector<const char*> kValidationLayerNames = {
                "VK_LAYER_KHRONOS_validation"
        };

        VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
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

            // Ignored messages
            {
                eastl::string message = _pCallbackData->pMessage;
                if (std::regex_match(message.begin(), message.end(), std::regex("Layer name .+ does not conform to naming standard .*"))
                    || std::regex_match(message.begin(), message.end(), std::regex("Override layer has override paths set to .*")))
                {
                    return VK_FALSE;
                }
            }

            if (_messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            {
                std::cout << "Validation layer (" << severity.c_str() << "): " << _pCallbackData->pMessage << std::endl;
            }

            KE_ERROR(_pCallbackData->pMessage);

            return VK_FALSE;
        }
    }

    VkGraphicsContext::VkGraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo, u64 _frameId)
        : m_appInfo(_appInfo)
    {
        if (m_appInfo.m_features.m_present)
        {
            m_window = eastl::make_unique<Window>(m_appInfo);
        }

        vk::ApplicationInfo applicationInfo(
                m_appInfo.m_applicationName.c_str(),
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
        _RetrieveOptionalExtensionNames(extensions, availableExtensions, m_appInfo);
        instanceCreateInfo.enabledExtensionCount = extensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

        VkAssert(vk::createInstance(&instanceCreateInfo, nullptr, &m_instance));

        _SetupValidationLayersCallback();

        if (m_appInfo.m_features.m_present)
        {
            m_surface = eastl::make_unique<VkSurface>(m_instance, m_window->GetGlfwWindow());
        }

        _SelectPhysicalDevice();

        if (m_appInfo.m_features.m_present)
        {
            m_surface->UpdateCapabilities(m_physicalDevice);
        }

        _CreateDevice();

#if !defined(KE_FINAL)
        m_debugHandler = eastl::make_shared<VkDebugHandler>();
        *m_debugHandler = VkDebugHandler::Initialize(m_device, m_debugUtils, m_debugMarkers);
        m_resources.m_debugHandler = m_debugHandler;
#endif

        if (m_appInfo.m_features.m_present)
        {
            m_swapChain = eastl::make_unique<VkSwapChain>(
                    m_appInfo,
                    m_device,
                    *m_surface,
                    m_resources,
                    m_window->GetGlfwWindow(),
                    m_queueIndices,
                    _frameId,
#if !defined(KE_FINAL)
                    m_debugHandler,
#else
                    nullptr,
#endif
                    nullptr);

            m_frameContextCount = m_swapChain->m_renderTargetViews.Size();
        }
        else
        {
            // If no display, keep double buffering.
            m_frameContextCount = 2;
        }

        m_frameContexts.Resize(m_frameContextCount);
        m_frameContexts.InitAll(m_device, m_queueIndices);
    }

    VkGraphicsContext::~VkGraphicsContext()
    {
        for (auto& frameContext: m_frameContexts)
        {
            frameContext.Destroy(m_device);
        }
        m_frameContexts.Clear();

        m_swapChain->Destroy(m_device, m_resources);
        m_swapChain.reset();

        m_surface->Destroy(m_instance);
        m_surface.reset();

        m_device.destroy();
        if (m_debugMessenger)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) m_instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(m_instance, m_debugMessenger, nullptr);
            }
        }
        m_instance.destroy();
    }

    void VkGraphicsContext::EndFrame(u64 _frameId)
    {
        const u8 frameIndex = _frameId % m_frameContextCount;
        auto& frameContext = m_frameContexts[frameIndex];
        eastl::fixed_vector<vk::Semaphore, 3> queueSemaphores;

        vk::Semaphore imageAvailableSemaphore;
        if (m_swapChain != nullptr)
        {
            imageAvailableSemaphore = m_swapChain->AcquireNextImage(m_device, frameIndex);
        }

        // Submit command buffers
	    {
            const auto submitQueue = [&](vk::Queue _queue, VkFrameContext::CommandPoolSet& _commandPoolSet)
            {
	            if (_queue && !_commandPoolSet.m_usedCommandBuffers.empty())
	            {
                    // Reset fence
                    {
                        KE_ASSERT(m_device.getFenceStatus(_commandPoolSet.m_fence) == vk::Result::eSuccess);
                        m_device.resetFences(_commandPoolSet.m_fence);
                    }

                    vk::SubmitInfo submitInfo;

                    constexpr vk::PipelineStageFlags stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
                    submitInfo.waitSemaphoreCount = m_swapChain != nullptr ? 1 : 0;
                    submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
                    submitInfo.pWaitDstStageMask = stages; // Only need image for render target output

                    submitInfo.commandBufferCount = _commandPoolSet.m_usedCommandBuffers.size();
                    submitInfo.pCommandBuffers = _commandPoolSet.m_usedCommandBuffers.data();
                    
                    submitInfo.signalSemaphoreCount = 1;
	            	submitInfo.pSignalSemaphores = &_commandPoolSet.m_semaphore;
                    queueSemaphores.push_back(_commandPoolSet.m_semaphore);

                    _queue.submit(submitInfo, _commandPoolSet.m_fence);
	            }
            };

            submitQueue(m_transferQueue, frameContext.m_transferCommandPoolSet);
            submitQueue(m_computeQueue, frameContext.m_computeCommandPoolSet);
            submitQueue(m_graphicsQueue, frameContext.m_graphicsCommandPoolSet);
	    }

        // Present image
        if (m_swapChain != nullptr)
    	{
            m_swapChain->Present(m_presentQueue, queueSemaphores, frameIndex);
        }
    }

    void VkGraphicsContext::WaitForFrame(u64 _frameId) const
    {
        const u8 frameIndex = _frameId % m_frameContextCount;
        m_frameContexts[frameIndex].WaitForFences(m_device, _frameId);
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

        if (KE_VERIFY(found))
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
            m_debugUtils = true;
        }

        if (_appInfo.m_features.m_debugTags == GraphicsCommon::SoftEnable::ForceEnabled)
        {
            result.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
            m_debugMarkers = true;
        }

        return result;
    }

    void VkGraphicsContext::_RetrieveOptionalExtensionNames(
            eastl::vector<const char *> &_currentList,
            const std::vector<vk::ExtensionProperties> &_extensionsAvailable,
            const GraphicsCommon::ApplicationInfo &_appInfo)
    {
        const auto find = [&_extensionsAvailable](const char* _extensionName)
        {
            const auto it = eastl::find(
                    _extensionsAvailable.begin(),
                    _extensionsAvailable.end(),
                    eastl::string(_extensionName),
                    [](vk::ExtensionProperties _a, const eastl::string& _b) {
                        return eastl::string(_a.extensionName.data()) == _b;
                    });
            return it != _extensionsAvailable.end();
        };

        if (_appInfo.m_features.m_debugTags == GraphicsCommon::SoftEnable::TryEnable)
        {
            if (find(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
            {
                _currentList.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
                m_debugMarkers = true;
            }
        }
    }

    vk::DebugUtilsMessengerCreateInfoEXT VkGraphicsContext::_PopulateDebugCreateInfo(void *_userData)
    {
        const auto severityFlags =
//                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
//                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
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

        if (KE_VERIFY(func != nullptr))
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

        if (KE_VERIFY_MSG(!suitableDevices.empty(), "No suitable device found!"))
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

        KE_ASSERT_MSG(features.m_transfer && (features.m_graphics || features.m_transferQueue), "Not supported yet");
        KE_ASSERT_MSG(features.m_compute && (features.m_graphics || features.m_asyncCompute), "Not supported yet");

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

        KE_ASSERT(_SelectQueues(m_appInfo, m_physicalDevice, m_surface->GetSurface(), m_queueIndices));
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

            createQueueInfo(m_queueIndices.m_graphicsQueueIndex, 1.0);
            createQueueInfo(m_queueIndices.m_transferQueueIndex, 0.5);
            createQueueInfo(m_queueIndices.m_computeQueueIndex, 0.5);
            createQueueInfo(m_queueIndices.m_presentQueueIndex, 1.0);

            for (u32 i = 0; i < queueCreateInfo.size(); i++)
            {
                KE_ASSERT(queueCreateInfo[i].queueCount == queuePriorities[i].size());
            }
        }

        vk::PhysicalDeviceFeatures features;

        const auto requiredExtensionsStrings = _GetRequiredDeviceExtensions();
        const auto requiredExtensions = StringHelpers::RetrieveStringPointerContainer(requiredExtensionsStrings);

        vk::ArrayProxyNoTemporaries<const char* const> enabledLayerNames;
        if (m_appInfo.m_features.m_validationLayers)
        {
            enabledLayerNames = MakeArrayProxy(kValidationLayerNames);
        }

        vk::DeviceCreateInfo createInfo({}, MakeArrayProxy(queueCreateInfo),
                                        enabledLayerNames,
                                        MakeArrayProxy(requiredExtensions),
                                        &features);

        VkAssert(m_physicalDevice.createDevice(&createInfo, nullptr, &m_device));

        _RetrieveQueues(m_queueIndices);
    }

    void VkGraphicsContext::_RetrieveQueues(const QueueIndices &_queueIndices)
    {
        const auto RetrieveQueue = [this](QueueIndices::Pair _queueIndex, vk::Queue& destQueue_)
        {
            if (!_queueIndex.IsInvalid())
            {
                destQueue_ = m_device.getQueue(_queueIndex.m_familyIndex, _queueIndex.m_indexInFamily);
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

    void VkGraphicsContext::BeginRenderPass(CommandList _commandList, GenPool::Handle _handle)
    {
        auto* renderPassData = m_resources.m_renderPasses.Get(_handle);
        VERIFY_OR_RETURN_VOID(renderPassData != nullptr);

        vk::RenderPassBeginInfo beginInfo {
            renderPassData->m_renderPass,
            renderPassData->m_framebuffer,
            { { 0, 0 },
                { renderPassData->m_size.m_width, renderPassData->m_size.m_height } },
            u32(renderPassData->m_clearValues.size()),
            renderPassData->m_clearValues.data()
        };

        _commandList.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
    }

    void VkGraphicsContext::EndRenderPass(CommandList _commandList)
    {
        _commandList.endRenderPass();
    }

    GenPool::Handle VkGraphicsContext::GetFrameContextPresentRenderTarget(u8 _index)
    {
        return m_swapChain->m_renderTargetViews[_index];
    }
}
