/**
 * @file
 * @author Max Godefroy
 * @date 19/02/2024.
 */

#pragma once

#include "VkHeaders.hpp"
#include <EASTL/shared_ptr.h>

namespace KryneEngine
{
    struct RenderTargetViewDesc;
    struct RenderPassDesc;

    class VkDebugHandler;

    struct VkResources
    {
        struct TextureColdData
        {
            Size16x2 m_size;
        };
        GenerationalPool<VkImage, TextureColdData> m_textures {};

        struct RTVColdData
        {
            VkFormat m_format;
            Size16x2 m_size;
        };
        GenerationalPool<VkImageView, RTVColdData> m_renderTargetViews {};

        struct RenderPassData
        {
            VkRenderPass m_renderPass;
            VkFramebuffer m_framebuffer;
            Size16x2 m_size;
            eastl::vector<VkClearValue> m_clearValues;
        };
        GenerationalPool<RenderPassData> m_renderPasses {};

#if !defined(KE_FINAL)
        eastl::shared_ptr<VkDebugHandler> m_debugHandler;
#endif

        // Default constructor and destructor, but moved implementation to cpp for .inl linking
        VkResources();
        ~VkResources();

        [[nodiscard]] GenPool::Handle RegisterTexture(VkImage _image, Size16x2 _size);

        bool ReleaseTexture(GenPool::Handle _handle, VkDevice _device, bool _free = true);

        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc, VkDevice& _device);

        bool FreeRenderTargetView(GenPool::Handle _handle, VkDevice _device);

        [[nodiscard]] GenPool::Handle CreateRenderPass(const RenderPassDesc& _desc, VkDevice _device);

        bool DestroyRenderPass(GenPool::Handle _handle, VkDevice _device);
    };
} // KryneEngine