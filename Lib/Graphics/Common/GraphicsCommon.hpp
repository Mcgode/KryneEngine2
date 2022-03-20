/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include "Common/KETypes.hpp"

namespace KryneEngine::GraphicsCommon
{
    struct Version
    {
        u16 m_major = 1;
        u16 m_minor = 0;
        u32 m_revision = 0;
    };

    enum class Api
    {
        Vulkan_1_0,
        Vulkan_1_1,
        Vulkan_1_2,
        Vulkan_1_3,

        VulkanStart = Vulkan_1_0,
        VulkanEnd = Vulkan_1_3,
    };

    struct ApplicationInfo
    {
        const char* m_applicationName = "Unnamed app";
        Version m_applicationVersion {};

        Version m_engineVersion { 1, 0, 0 };
        Api m_api;

        struct Features
        {
            bool m_validationLayers = true;
        }
        m_features {};
    };
}