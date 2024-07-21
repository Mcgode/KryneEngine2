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
        VkFrameContext(VkDevice _device, const VkCommonStructures::QueueIndices& _queueIndices);

        virtual ~VkFrameContext();

#if !defined(KE_FINAL)
        void SetDebugHandler(const eastl::shared_ptr<VkDebugHandler> &_debugHandler, VkDevice _device, u8 _frameIndex);
#endif

        void Destroy(VkDevice _device);

        VkCommandBuffer BeginGraphicsCommandBuffer(VkDevice _device)
        {
            return m_graphicsCommandPoolSet.BeginCommandBuffer(_device);
        }

        void EndGraphicsCommandBuffer()
        {
            m_graphicsCommandPoolSet.EndCommandBuffer();
        }

        VkCommandBuffer BeginComputeCommandBuffer(VkDevice _device)
        {
            return m_computeCommandPoolSet.BeginCommandBuffer(_device);
        }

        void EndComputeCommandBuffer()
        {
            m_computeCommandPoolSet.EndCommandBuffer();
        }

        VkCommandBuffer BeginTransferCommandBuffer(VkDevice _device)
        {
            return m_transferCommandPoolSet.BeginCommandBuffer(_device);
        }

        void EndTransferCommandBuffer()
        {
            m_transferCommandPoolSet.EndCommandBuffer();
        }

        void WaitForFences(VkDevice _device, u64 _frameId) const;

    private:
        struct CommandPoolSet
        {
            VkCommandPool m_commandPool = VK_NULL_HANDLE;

            eastl::vector<VkCommandBuffer> m_availableCommandBuffers;
            eastl::vector<VkCommandBuffer> m_usedCommandBuffers;

            LightweightMutex m_mutex {};

            VkFence m_fence = VK_NULL_HANDLE;
            VkSemaphore m_semaphore = VK_NULL_HANDLE;

#if !defined(KE_FINAL)
            eastl::shared_ptr<VkDebugHandler> m_debugHandler;
            eastl::string m_baseDebugString;

            void SetDebugHandler(const eastl::shared_ptr<VkDebugHandler>& _handler, VkDevice _device, const eastl::string_view &_baseString);
#endif

            VkCommandBuffer BeginCommandBuffer(VkDevice _device);
            void EndCommandBuffer();

            void Reset();

            void Destroy(VkDevice _device);
        };

        static constexpr u8 kMaxQueueCount = 3;

        CommandPoolSet m_graphicsCommandPoolSet;
        CommandPoolSet m_computeCommandPoolSet;
        CommandPoolSet m_transferCommandPoolSet;
        eastl::fixed_vector<VkFence, kMaxQueueCount> m_fencesArray;
        u64 m_frameId = 0;
    };
} // KryneEngine