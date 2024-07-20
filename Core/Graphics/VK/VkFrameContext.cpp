/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#include "VkFrameContext.hpp"
#include "HelperFunctions.hpp"

namespace KryneEngine
{
    VkFrameContext::VkFrameContext(vk::Device _device, const VkCommonStructures::QueueIndices &_queueIndices)
    {
        const auto CreateCommandPool = [this, _device](
                const VkCommonStructures::QueueIndices::Pair& _pair,
                CommandPoolSet& _commandPoolSet)
        {
            if (!_pair.IsInvalid())
            {
                // Create command pool
	            {
		            const vk::CommandPoolCreateInfo createInfo {
                        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                        static_cast<u32>(_pair.m_familyIndex)
		            };

                    _commandPoolSet.m_commandPool = _device.createCommandPool(createInfo);
	            }

                // Create fence
	            {
		            constexpr vk::FenceCreateInfo createInfo {
		            	vk::FenceCreateFlagBits::eSignaled // Create as signaled for first wait
					};
                    _commandPoolSet.m_fence = _device.createFence(createInfo);

                    // Save fences into single array for mutualized waits and resets
                    m_fencesArray.push_back(_commandPoolSet.m_fence);
	            }

                // Create semaphore
	            {
                    constexpr vk::SemaphoreCreateInfo createInfo = {};
                    _commandPoolSet.m_semaphore = _device.createSemaphore(createInfo);
	            }
            }
        };

        CreateCommandPool(_queueIndices.m_graphicsQueueIndex, m_graphicsCommandPoolSet);
        CreateCommandPool(_queueIndices.m_computeQueueIndex, m_computeCommandPoolSet);
        CreateCommandPool(_queueIndices.m_transferQueueIndex, m_transferCommandPoolSet);
    }

    VkFrameContext::~VkFrameContext()
    {
        KE_ASSERT(!m_graphicsCommandPoolSet.m_commandPool);
        KE_ASSERT(!m_computeCommandPoolSet.m_commandPool);
        KE_ASSERT(!m_transferCommandPoolSet.m_commandPool);
    }

    void VkFrameContext::Destroy(vk::Device _device)
    {
        m_graphicsCommandPoolSet.Destroy(_device);
        m_computeCommandPoolSet.Destroy(_device);
        m_transferCommandPoolSet.Destroy(_device);
    }

    void VkFrameContext::WaitForFences(vk::Device _device, u64 _frameId) const
    {
        // If fences have already been reset to a later frame, then previous fence was signaled, no need to wait.
        if (m_frameId > _frameId)
        {
            return;
        }

        VkAssert(_device.waitForFences(m_fencesArray.size(), m_fencesArray.data(), true, UINT64_MAX));
    }

    vk::CommandBuffer VkFrameContext::CommandPoolSet::BeginCommandBuffer(vk::Device _device)
    {
        m_mutex.ManualLock();

        if (m_availableCommandBuffers.empty())
        {
            const vk::CommandBufferAllocateInfo allocateInfo {
                m_commandPool,
                vk::CommandBufferLevel::ePrimary,
                1,
            };
            VkAssert(_device.allocateCommandBuffers(&allocateInfo, &m_usedCommandBuffers.push_back()));
        }
        else
        {
            m_usedCommandBuffers.push_back(m_availableCommandBuffers.back());
            m_availableCommandBuffers.pop_back();
        }

        auto commandBuffer = m_usedCommandBuffers.back();

        const vk::CommandBufferBeginInfo beginInfo { vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
        VkAssert(commandBuffer.begin(&beginInfo));

        return commandBuffer;
    }

    void VkFrameContext::CommandPoolSet::EndCommandBuffer()
    {
        m_usedCommandBuffers.back().end();

        m_mutex.ManualUnlock();
    }

    void VkFrameContext::CommandPoolSet::Reset()
    {
        const auto lock = m_mutex.AutoLock();

        for (auto& commandBuffer: m_usedCommandBuffers)
        {
            commandBuffer.reset();
        }

        m_availableCommandBuffers.insert(
                m_availableCommandBuffers.end(),
                m_usedCommandBuffers.begin(),
                m_usedCommandBuffers.end());
        m_usedCommandBuffers.clear();
    }

    void VkFrameContext::CommandPoolSet::Destroy(vk::Device _device)
    {
        SafeDestroy(_device, m_semaphore);

        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetFenceStatus.html
        KE_ASSERT_MSG(!m_fence || _device.getFenceStatus(m_fence) == vk::Result::eSuccess, "Fence should be signaled by the time the frame is destroyed");
        SafeDestroy(_device, m_fence);

        const auto lock = m_mutex.AutoLock();
        KE_ASSERT_MSG(m_usedCommandBuffers.empty(), "PoolSet should be reset before destroy");

        if (!m_usedCommandBuffers.empty())
        {
            _device.free(m_commandPool, m_usedCommandBuffers);
        }
        if (!m_availableCommandBuffers.empty())
        {
            _device.free(m_commandPool, m_availableCommandBuffers);
        }

        SafeDestroy(_device, m_commandPool);
    }
} // KryneEngine