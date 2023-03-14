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
                vk::CommandPoolCreateInfo createInfo {};
                createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
                createInfo.queueFamilyIndex = _pair.m_familyIndex;

                VkAssert(m_deviceRef->createCommandPool(&createInfo, nullptr, &_commandPoolSet.m_commandPool));
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

    vk::CommandBuffer VkFrameContext::CommandPoolSet::BeginCommandBuffer(VkSharedDeviceRef& _deviceRef)
    {
        m_mutex.ManualLock();

        if (m_availableCommandBuffers.empty())
        {
            vk::CommandBufferAllocateInfo allocateInfo { m_commandPool };
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