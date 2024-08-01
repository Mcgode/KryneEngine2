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

        const VkApplicationInfo applicationInfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = m_appInfo.m_applicationName.c_str(),
            .applicationVersion = MakeVersion(m_appInfo.m_applicationVersion),
            .pEngineName = "KryneEngine2",
            .engineVersion = MakeVersion(m_appInfo.m_engineVersion),
            .apiVersion = GetApiVersion(m_appInfo.m_api)
        };

        VkInstanceCreateInfo instanceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .flags = 0,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = 0,
        };

        DynamicArray<VkExtensionProperties> availableExtensions;
        VkHelperFunctions::VkArrayFetch(availableExtensions, vkEnumerateInstanceExtensionProperties, nullptr);
        std::cout << "Available extensions:" << std::endl;
        for (const auto& extension: availableExtensions)
        {
            std::cout << "\t" << extension.extensionName << std::endl;
        }

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
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

        VkAssert(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance));

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
        *m_debugHandler = VkDebugHandler::Initialize(m_device, m_appInfo, m_debugUtils, m_debugMarkers);
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
                    nullptr);

#if !defined(KE_FINAL)
            m_swapChain->SetDebugHandler(m_debugHandler, m_device);
#endif

            m_frameContextCount = m_swapChain->m_renderTargetViews.Size();
        }
        else
        {
            // If no display, keep double buffering.
            m_frameContextCount = 2;
        }

        m_frameContexts.Resize(m_frameContextCount);
        m_frameContexts.InitAll(m_device, m_queueIndices);

#if !defined(KE_FINAL)
        for (auto i = 0u; i < m_frameContextCount; i++)
        {
            m_frameContexts[i].SetDebugHandler(m_debugHandler, m_device, i);
        }
#endif
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

        vkDestroyDevice(m_device, nullptr);
        if (m_debugMessenger)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(m_instance, m_debugMessenger, nullptr);
            }
        }
        vkDestroyInstance(m_instance, nullptr);
    }

    void VkGraphicsContext::EndFrame(u64 _frameId)
    {
        const u8 frameIndex = _frameId % m_frameContextCount;
        auto& frameContext = m_frameContexts[frameIndex];
        eastl::fixed_vector<VkSemaphore, VkFrameContext::kMaxQueueCount> queueSemaphores;

        VkSemaphore imageAvailableSemaphore;
        if (m_swapChain != nullptr)
        {
            imageAvailableSemaphore = m_swapChain->m_imageAvailableSemaphores[frameIndex];
        }

        // Submit command buffers
	    {
            const auto submitQueue = [&](VkQueue _queue, VkFrameContext::CommandPoolSet& _commandPoolSet)
            {
	            if (_queue && !_commandPoolSet.m_usedCommandBuffers.empty())
	            {
                    // Reset fence
                    {
                        KE_ASSERT(vkGetFenceStatus(m_device, _commandPoolSet.m_fence) == VK_SUCCESS);
                        VkAssert(vkResetFences(m_device, 1, &_commandPoolSet.m_fence));
                    }

                    constexpr VkPipelineStageFlags stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
                    VkSubmitInfo submitInfo
                    {
                        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,

                        .waitSemaphoreCount = m_swapChain != nullptr ? 1u : 0u,
                        .pWaitSemaphores = &imageAvailableSemaphore,
                        .pWaitDstStageMask = stages, // Only need image for render target output

                        .commandBufferCount = static_cast<uint32_t>(_commandPoolSet.m_usedCommandBuffers.size()),
                        .pCommandBuffers = _commandPoolSet.m_usedCommandBuffers.data(),

                        .signalSemaphoreCount = 1,
                        .pSignalSemaphores = &_commandPoolSet.m_semaphore,
                    };
                    queueSemaphores.push_back(_commandPoolSet.m_semaphore);

                    VkAssert(vkQueueSubmit(_queue, 1, &submitInfo, _commandPoolSet.m_fence));
                }
            };

            submitQueue(m_transferQueue, frameContext.m_transferCommandPoolSet);
            submitQueue(m_computeQueue, frameContext.m_computeCommandPoolSet);
            submitQueue(m_graphicsQueue, frameContext.m_graphicsCommandPoolSet);
	    }

        // Present image
        if (m_swapChain != nullptr) {
            m_swapChain->Present(m_presentQueue, queueSemaphores);
        }

        const u64 nextFrameId = _frameId + 1;
        const u8 nextFrameContextIndex = nextFrameId % m_frameContextCount;
        if (nextFrameId >= m_frameContextCount)
        {
            auto&nextFrameContext = m_frameContexts[nextFrameContextIndex];
            nextFrameContext.WaitForFences(m_device, nextFrameId - m_frameContextCount);
            nextFrameContext.m_graphicsCommandPoolSet.Reset();
            nextFrameContext.m_computeCommandPoolSet.Reset();
            nextFrameContext.m_transferCommandPoolSet.Reset();
        }

        // Acquire next image
        if (m_swapChain != nullptr)
        {
            m_swapChain->AcquireNextImage(m_device, nextFrameContextIndex);
        }
    }

    bool VkGraphicsContext::IsFrameExecuted(KryneEngine::u64 _frameId) const
    {
        const u8 frameIndex = _frameId % m_frameContextCount;
        return m_frameContexts[frameIndex].m_frameId > _frameId;
    }

    void VkGraphicsContext::WaitForFrame(u64 _frameId) const
    {
        const u8 frameIndex = _frameId % m_frameContextCount;
        m_frameContexts[frameIndex].WaitForFences(m_device, _frameId);
    }

    void VkGraphicsContext::_PrepareValidationLayers(VkInstanceCreateInfo& _createInfo)
    {
         DynamicArray<VkLayerProperties> availableLayers;
         VkHelperFunctions::VkArrayFetch(availableLayers, vkEnumerateInstanceLayerProperties);

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
            const DynamicArray<VkExtensionProperties> &_availableExtensions,
            const GraphicsCommon::ApplicationInfo &_appInfo)
    {
        const auto find = [&_availableExtensions](const char* _extensionName)
        {
            const auto it = eastl::find(
                    _availableExtensions.begin(),
                    _availableExtensions.end(),
                    eastl::string(_extensionName),
                    [](VkExtensionProperties _a, const eastl::string& _b) {
                        return eastl::string(_a.extensionName) == _b;
                    });
            return it != _availableExtensions.end();
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

    VkDebugUtilsMessengerCreateInfoEXT VkGraphicsContext::_PopulateDebugCreateInfo(void *_userData)
    {
        const auto severityFlags =
//                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
//                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        const auto messageTypeFlags =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

        return {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .flags = 0,
                .messageSeverity = severityFlags,
                .messageType = messageTypeFlags,
                .pfnUserCallback = DebugCallback,
                .pUserData = _userData
        };
    }

    void VkGraphicsContext::_SetupValidationLayersCallback()
    {
        auto createInfo = _PopulateDebugCreateInfo(this);
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");

        if (KE_VERIFY(func != nullptr))
        {
            VkAssert(func(m_instance, &createInfo,
                          nullptr, &m_debugMessenger));
        }
    }

    void VkGraphicsContext::_SelectPhysicalDevice()
    {
        DynamicArray<VkPhysicalDevice> availablePhysicalDevices;
        VkHelperFunctions::VkArrayFetch(availablePhysicalDevices, vkEnumeratePhysicalDevices, m_instance);

        eastl::vector<VkPhysicalDevice> suitableDevices;
        eastl::copy_if(availablePhysicalDevices.begin(), availablePhysicalDevices.end(),
                       eastl::back_inserter(suitableDevices),
                       [this](const VkPhysicalDevice& _physicalDevice)
        {
            DynamicArray<VkExtensionProperties> extensions;
            VkHelperFunctions::VkArrayFetch(extensions, vkEnumerateDeviceExtensionProperties, _physicalDevice, nullptr);
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
            VkPhysicalDevice selectedDevice;

            for (const auto& suitableDevice: suitableDevices)
            {
                u32 score = 0;
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(suitableDevice, &properties);
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
                                     const VkPhysicalDevice &_physicalDevice,
                                     const VkSurfaceKHR &_surface,
                                     QueueIndices &_indices)
    {
        DynamicArray<VkQueueFamilyProperties> familyProperties;
        VkHelperFunctions::VkArrayFetch(familyProperties, vkGetPhysicalDeviceQueueFamilyProperties, _physicalDevice);
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
            for (s8 i = 0; i < familyProperties.Size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;

                const bool graphicsOk = bool(flags & VK_QUEUE_GRAPHICS_BIT);
                const bool transferOk = !features.m_transfer || features.m_transferQueue || bool(flags & VK_QUEUE_TRANSFER_BIT);
                const bool computeOk = !features.m_compute || features.m_asyncCompute || bool(flags & VK_QUEUE_COMPUTE_BIT);

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
            for (s8 i = 0; i < familyProperties.Size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;
                if (flags & VK_QUEUE_TRANSFER_BIT && GetIndexOfFamily(i) < familyProperties[i].queueCount)
                {
                    u8 score = 0;
                    score += flags & VK_QUEUE_GRAPHICS_BIT ? 0 : 4;
                    score += flags & VK_QUEUE_COMPUTE_BIT ? 0 : 3;

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
            for (s8 i = 0; i < familyProperties.Size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;
                if (flags & VK_QUEUE_COMPUTE_BIT && GetIndexOfFamily(i) < familyProperties[i].queueCount)
                {
                    u8 score = 0;
                    score += flags & VK_QUEUE_TRANSFER_BIT ? 0 : 1;
                    score += flags & VK_QUEUE_GRAPHICS_BIT ? 0 : 3;

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
            for (s8 i = 0; i < familyProperties.Size(); i++)
            {
                const auto flags = familyProperties[i].queueFlags;
                VkBool32 supported;
                vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &supported);
                if (supported && GetIndexOfFamily(i) < familyProperties[i].queueCount)
                {
                    u8 score = 0;
                    score += flags & VK_QUEUE_GRAPHICS_BIT ? 1 : 5;
                    score += flags & VK_QUEUE_TRANSFER_BIT ? 1 : 4;
                    score += flags & VK_QUEUE_COMPUTE_BIT ? 1 : 3;

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
        eastl::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
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
                                      [](const VkDeviceQueueCreateInfo& _info, u32 _familyIndex)
                                      { return _info.queueFamilyIndex == _familyIndex; });
                bool alreadyInserted = it != queueCreateInfo.end();

                if (!alreadyInserted)
                {
                    it = queueCreateInfo.emplace(queueCreateInfo.end());
                    queuePriorities.push_back();
                    it->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    it->flags = 0;
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

        VkPhysicalDeviceFeatures features;
        // Init struct data;
        memset(&features, VK_FALSE, sizeof(VkPhysicalDeviceFeatures));

        const auto requiredExtensionsStrings = _GetRequiredDeviceExtensions();
        const auto requiredExtensions = StringHelpers::RetrieveStringPointerContainer(requiredExtensionsStrings);

        VkDeviceCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .flags = 0,
            .queueCreateInfoCount = static_cast<u32>(queueCreateInfo.size()),
            .pQueueCreateInfos = queueCreateInfo.data(),
            .enabledLayerCount = static_cast<u32>(kValidationLayerNames.size()),
            .ppEnabledLayerNames = kValidationLayerNames.data(),
            .enabledExtensionCount = static_cast<u32>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data(),
            .pEnabledFeatures = &features
        };

        VkAssert(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));

        _RetrieveQueues(m_queueIndices);
    }

    void VkGraphicsContext::_RetrieveQueues(const QueueIndices &_queueIndices)
    {
        const auto RetrieveQueue = [this](QueueIndices::Pair _queueIndex, VkQueue& destQueue_)
        {
            if (!_queueIndex.IsInvalid())
            {
                vkGetDeviceQueue(m_device, _queueIndex.m_familyIndex, _queueIndex.m_indexInFamily, &destQueue_);
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

        VkRenderPassBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderPassData->m_renderPass,
            .framebuffer = renderPassData->m_framebuffer,
            .renderArea = {
                .offset = { 0, 0 },
                .extent = { renderPassData->m_size.m_width, renderPassData->m_size.m_height }
            },
            .clearValueCount = u32(renderPassData->m_clearValues.size()),
            .pClearValues = renderPassData->m_clearValues.data()
        };

        vkCmdBeginRenderPass(_commandList, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VkGraphicsContext::EndRenderPass(CommandList _commandList)
    {
        vkCmdEndRenderPass(_commandList);
    }

    eastl::vector<TextureMemoryFootprint> VkGraphicsContext::FetchTextureSubResourcesMemoryFootprints(
        const TextureDesc& _desc)
    {
        eastl::vector<TextureMemoryFootprint> footprints;

        u64 cumulatedOffset = 0;
        for (u32 sliceIndex = 0; sliceIndex < _desc.m_arraySize; sliceIndex++)
        {
            for (u32 mipIndex = 0; mipIndex < _desc.m_mipCount; mipIndex++)
            {
                TextureMemoryFootprint footprint {
                    .m_offset = cumulatedOffset,
                    .m_width = eastl::max(_desc.m_dimensions.x >> mipIndex, 1u),
                    .m_height = eastl::max(_desc.m_dimensions.y >> mipIndex, 1u),
                    .m_depth = static_cast<u16>(eastl::max(_desc.m_dimensions.z >> mipIndex, 1u)),
                    .m_format = _desc.m_format,
                };
                const u32 sizePerBlock = VkHelperFunctions::GetByteSizePerBlock(VkHelperFunctions::ToVkFormat(_desc.m_format));
                footprint.m_lineByteAlignedSize = sizePerBlock * footprint.m_width;

                const u64 size = footprint.m_lineByteAlignedSize * footprint.m_height * footprint.m_depth;
                cumulatedOffset += size;
            }
        }

        return footprints;
    }

    GenPool::Handle VkGraphicsContext::GetPresentRenderTarget(u8 _index)
    {
        return (m_swapChain != nullptr)
                ? m_swapChain->m_renderTargetViews[_index]
                : GenPool::kInvalidHandle;
    }

    u32 VkGraphicsContext::GetCurrentPresentImageIndex() const
    {
        return (m_swapChain != nullptr)
                ? m_swapChain->m_imageIndex
                : 0;
    }
}
