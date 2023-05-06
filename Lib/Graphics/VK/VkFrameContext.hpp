/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#pragma once

#include "VkHeaders.hpp"
#include "CommonStructures.hpp"
#include <Threads/LightweightMutex.hpp>

#include "EASTL/fixed_vector.h"

namespace KryneEngine
{
    class VkFrameContext
    {
        friend class VkGraphicsContext;

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

        void WaitForFences(u64 _frameId);

    private:
        VkSharedDeviceRef m_deviceRef;

        struct CommandPoolSet
        {
            vk::CommandPool m_commandPool;

            eastl::vector<vk::CommandBuffer> m_availableCommandBuffers;
            eastl::vector<vk::CommandBuffer> m_usedCommandBuffers;

            LightweightMutex m_mutex {};

            vk::Fence m_fence;
            vk::Semaphore m_semaphore;

            vk::CommandBuffer BeginCommandBuffer(VkSharedDeviceRef& _deviceRef);
            void EndCommandBuffer();

            void Destroy(VkSharedDeviceRef& _deviceRef);
        };

        CommandPoolSet m_graphicsCommandPoolSet;
        CommandPoolSet m_computeCommandPoolSet;
        CommandPoolSet m_transferCommandPoolSet;
        eastl::fixed_vector<vk::Fence, 3> m_fencesArray;
        u64 m_frameId = 0;
    };
} // KryneEngine