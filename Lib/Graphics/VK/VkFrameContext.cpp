/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#include "VkFrameContext.hpp"
#include "HelperFunctions.hpp"

namespace KryneEngine
{
    using VkHelperFunctions::VkAssert;

    VkFrameContext::VkFrameContext(VkSharedDevice *_device, const VkCommonStructures::QueueIndices &_queueIndices)
        : m_deviceRef(eastl::move(_device->MakeRef()))
    {
        const auto CreateCommandPool = [this](
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

                    _commandPoolSet.m_commandPool = m_deviceRef->createCommandPool(createInfo);
	            }

                // Create fence
	            {
		            constexpr vk::FenceCreateInfo createInfo {
		            	vk::FenceCreateFlagBits::eSignaled // Create as signaled for first wait
					};
                    _commandPoolSet.m_fence = m_deviceRef->createFence(createInfo);

                    // Save fences into single array for mutualized waits and resets
                    m_fencesArray.push_back(_commandPoolSet.m_fence);
	            }

                // Create semaphore
	            {
                    constexpr vk::SemaphoreCreateInfo createInfo = {};
                    _commandPoolSet.m_semaphore = m_deviceRef->createSemaphore(createInfo);
	            }
            }
        };

        CreateCommandPool(_queueIndices.m_graphicsQueueIndex, m_graphicsCommandPoolSet);
        CreateCommandPool(_queueIndices.m_computeQueueIndex, m_computeCommandPoolSet);
        CreateCommandPool(_queueIndices.m_transferQueueIndex, m_transferCommandPoolSet);
    }

    VkFrameContext::~VkFrameContext()
    {
        m_graphicsCommandPoolSet.Destroy(m_deviceRef);
        m_computeCommandPoolSet.Destroy(m_deviceRef);
        m_transferCommandPoolSet.Destroy(m_deviceRef);
    }

    void VkFrameContext::WaitForFences(u64 _frameId)
    {
        // If fences have already been reset to a later frame, then previous fence was signaled, no need to wait.
        if (m_frameId > _frameId)
        {
            return;
        }

        VkAssert(m_deviceRef->waitForFences(m_fencesArray.size(), m_fencesArray.data(), true, UINT64_MAX));
    }

    vk::CommandBuffer VkFrameContext::CommandPoolSet::BeginCommandBuffer(VkSharedDeviceRef& _deviceRef)
    {
        m_mutex.ManualLock();

        if (m_availableCommandBuffers.empty())
        {
	        const vk::CommandBufferAllocateInfo allocateInfo { m_commandPool };
            VkAssert(_deviceRef->allocateCommandBuffers(&allocateInfo, &m_availableCommandBuffers.push_back()));
        }
        else
        {
            m_usedCommandBuffers.push_back(m_availableCommandBuffers.back());
            m_availableCommandBuffers.pop_back();
        }

        auto commandBuffer = m_usedCommandBuffers.back();

        VkAssert(commandBuffer.begin({}));

        return commandBuffer;
    }

    void VkFrameContext::CommandPoolSet::EndCommandBuffer()
    {
        m_usedCommandBuffers.back().end();

        m_mutex.ManualUnlock();
    }

    void VkFrameContext::CommandPoolSet::Destroy(VkSharedDeviceRef &_deviceRef)
    {
        _deviceRef->destroy(m_semaphore);

        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetFenceStatus.html
        Assert(!m_fence || _deviceRef->getFenceStatus(m_fence) == vk::Result::eSuccess, "Fence should be signaled by the time the frame is destroyed");
        _deviceRef->destroy(m_fence);

        const auto lock = m_mutex.AutoLock();
        Assert(m_usedCommandBuffers.empty(), "PoolSet should be reset before destroy");

        if (!m_usedCommandBuffers.empty())
        {
            _deviceRef->free(m_commandPool, m_usedCommandBuffers);
        }
        if (!m_availableCommandBuffers.empty())
        {
            _deviceRef->free(m_commandPool, m_availableCommandBuffers);
        }

        _deviceRef->destroy(m_commandPool);
    }
} // KryneEngine