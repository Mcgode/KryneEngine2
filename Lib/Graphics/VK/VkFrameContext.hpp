/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#pragma once

#include "VkHeaders.hpp"
#include "CommonStructures.hpp"
#include <Threads/LightweightMutex.hpp>

namespace KryneEngine
{
    class VkFrameContext
    {
    public:
        VkFrameContext(VkSharedDevice* _device, const VkCommonStructures::QueueIndices& _queueIndices);

        virtual ~VkFrameContext();

        vk::CommandBuffer BeginGraphicsCommandBuffer()
        {
            return m_graphicsCommandPoolSet.BeginCommandBuffer(m_deviceRef);
        }

        void EndGraphicsCommandBuffer()
        {
            m_graphicsCommandPoolSet.EndCommandBuffer();
        }

        vk::CommandBuffer BeginComputeCommandBuffer()
        {
            return m_computeCommandPoolSet.BeginCommandBuffer(m_deviceRef);
        }

        void EndComputeCommandBuffer()
        {
            m_computeCommandPoolSet.EndCommandBuffer();
        }

        vk::CommandBuffer BeginTransferCommandBuffer()
        {
            return m_transferCommandPoolSet.BeginCommandBuffer(m_deviceRef);
        }

        void EndTransferCommandBuffer()
        {
            m_transferCommandPoolSet.EndCommandBuffer();
        }

    private:
        VkSharedDeviceRef m_deviceRef;

        struct CommandPoolSet
        {
            vk::CommandPool m_commandPool;

            eastl::vector<vk::CommandBuffer> m_availableCommandBuffers;
            eastl::vector<vk::CommandBuffer> m_usedCommandBuffers;

            LightweightMutex m_mutex {};

            vk::CommandBuffer BeginCommandBuffer(VkSharedDeviceRef& _deviceRef);
            void EndCommandBuffer();

            void Destroy(VkSharedDeviceRef& _deviceRef);
        };

        CommandPoolSet m_graphicsCommandPoolSet;
        CommandPoolSet m_computeCommandPoolSet;
        CommandPoolSet m_transferCommandPoolSet;
    };
} // KryneEngine