/**
 * @file
 * @author Max Godefroy
 * @date 10/07/2022.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <Graphics/Common/TextureMemory.hpp>
#include <Graphics/VK/CommonStructures.hpp>

namespace KryneEngine
{
    class VkTextureMemory: public TextureMemory
    {
    public:
        VkTextureMemory(vk::Image _systemOwnedImage, u64 _memorySize);

        u64 GetMemorySize() const override { return m_memorySize; }

    protected:
        ~VkTextureMemory() override;

        void _Destroy() override;

    private:
        VkSharedDeviceRef m_deviceRef;
        vk::Image m_image;
        u64 m_memorySize;
    };
} // KryneEngine