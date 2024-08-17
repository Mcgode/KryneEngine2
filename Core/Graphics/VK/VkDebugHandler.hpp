/**
 * @file
 * @author Max Godefroy
 * @date 17/07/2024.
 */

#pragma once

#include "HelperFunctions.hpp"
#include <Common/Types.hpp>
#include <vulkan/vulkan.h>

namespace KryneEngine
{
    class VkDebugHandler
    {
    public:
        static VkDebugHandler Initialize(VkDevice _device, bool _debugUtilsEnabled, bool _debugMarkersEnabled)
        {
            ZoneScopedN("VkDebugHandler init");

            VkDebugHandler handler;

            if (_debugUtilsEnabled)
            {
                handler.m_setObjectDebugNameFunc = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
                    vkGetDeviceProcAddr(_device, "vkSetDebugUtilsObjectNameEXT"));
            }

            if (_debugMarkersEnabled)
            {
                handler.m_setObjectMarkerNameFunc = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(
                    vkGetDeviceProcAddr(_device, "vkDebugMarkerSetObjectNameEXT"));
            }

            return handler;
        }

        VkResult SetName(VkDevice _device, VkObjectType _objectType, u64 _objectHandle, const eastl::string_view& _name)
        {
            VkResult result = VK_INCOMPLETE;

            const eastl::string name = _name.data();

            if (_objectHandle == (u64)VK_NULL_HANDLE)
            {
                return result;
            }

            if (m_setObjectDebugNameFunc != nullptr)
            {
                VkDebugUtilsObjectNameInfoEXT nameInfo {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .objectType = _objectType,
                    .objectHandle = _objectHandle,
                    .pObjectName = name.c_str()
                };

                result = m_setObjectDebugNameFunc(_device, &nameInfo);
            }

            if (m_setObjectMarkerNameFunc != nullptr)
            {
                VkDebugMarkerObjectNameInfoEXT nameInfo {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
                    .objectType = VkHelperFunctions::ConvertObjectType(_objectType),
                    .object = _objectHandle,
                    .pObjectName = name.c_str()
                };

                result = m_setObjectMarkerNameFunc(_device, &nameInfo);
            }

            return result;
        }

    private:
        PFN_vkSetDebugUtilsObjectNameEXT  m_setObjectDebugNameFunc = nullptr;
        PFN_vkDebugMarkerSetObjectNameEXT m_setObjectMarkerNameFunc = nullptr;
    };
}