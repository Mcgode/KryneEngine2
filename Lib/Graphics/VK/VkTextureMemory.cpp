/**
 * @file
 * @author Max Godefroy
 * @date 10/07/2022.
 */

#include "VkTextureMemory.hpp"
#include <Common/Assert.hpp>

namespace KryneEngine
{
    VkTextureMemory::VkTextureMemory(vk::Image _systemOwnedImage, u64 _memorySize)
        : TextureMemory(true)
        , m_image(_systemOwnedImage)
        , m_memorySize(_memorySize)
    {}

    VkTextureMemory::~VkTextureMemory()
    {
        _BeforeDestruct();
    }

    void VkTextureMemory::_Destroy()
    {
        VERIFY_OR_RETURN_VOID(!m_manualDestroy && m_deviceRef != nullptr);

        m_deviceRef->destroy(m_image);
        m_image = VK_NULL_HANDLE;
    }
} // KryneEngine