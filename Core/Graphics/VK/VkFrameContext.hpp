/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#pragma once

#include "VkHeaders.hpp"
#include "CommonStructures.hpp"
#include <Threads/LightweightMutex.hpp>
#include <EASTL/shared_ptr.h>

#include "EASTL/fixed_vector.h"

namespace KryneEngine
{
    class VkDebugHandler;

    class VkFrameContext
    {
        friend class VkGraphicsContext;

    public:
        VkFrameContext(vk::Device _device, const VkCommonStructures::QueueIndices& _queueIndices);

        virtual ~VkFrameContext();

#if !defined(KE_FINAL)
        void SetDebugHandler (const eastl::shared_ptr<VkDebugHandler> &_debugHandler, vk::Device _device, u8 _frameIndex);
#endif

        void Destroy(vk::Device _device);

        vk::CommandBuffer BeginGraphicsCommandBuffer(vk::Device _device)
        {
            return m_graphicsCommandPoolSet.BeginCommandBuffer(_device);
        }

        void EndGraphicsCommandBuffer()
        {
            m_graphicsCommandPoolSet.EndCommandBuffer();
        }

        vk::CommandBuffer BeginComputeCommandBuffer(vk::Device _device)
        {
            return m_computeCommandPoolSet.BeginCommandBuffer(_device);
        }

        void EndComputeCommandBuffer()
        {
            m_computeCommandPoolSet.EndCommandBuffer();
        }

        vk::CommandBuffer BeginTransferCommandBuffer(vk::Device _device)
        {
            return m_transferCommandPoolSet.BeginCommandBuffer(_device);
        }

        void EndTransferCommandBuffer()
        {
            m_transferCommandPoolSet.EndCommandBuffer();
        }

        void WaitForFences(vk::Device _device, u64 _frameId) const;

    private:
        struct CommandPoolSet
        {
            vk::CommandPool m_commandPool;

            eastl::vector<vk::CommandBuffer> m_availableCommandBuffers;
            eastl::vector<vk::CommandBuffer> m_usedCommandBuffers;

            LightweightMutex m_mutex {};

            vk::Fence m_fence;
            vk::Semaphore m_semaphore;

#if !defined(KE_FINAL)
            eastl::shared_ptr<VkDebugHandler> m_debugHandler;
            eastl::string m_baseDebugString;

            void SetDebugHandler(const eastl::shared_ptr<VkDebugHandler>& _handler, vk::Device _device, const eastl::string_view &_baseString);
#endif

            vk::CommandBuffer BeginCommandBuffer(vk::Device _device);
            void EndCommandBuffer();

            void Reset();

            void Destroy(vk::Device _device);
        };

        static constexpr u8 kMaxQueueCount = 3;

        CommandPoolSet m_graphicsCommandPoolSet;
        CommandPoolSet m_computeCommandPoolSet;
        CommandPoolSet m_transferCommandPoolSet;
        eastl::fixed_vector<vk::Fence, kMaxQueueCount> m_fencesArray;
        u64 m_frameId = 0;
    };
} // KryneEngine