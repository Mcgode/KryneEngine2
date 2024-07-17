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
        GenerationalPool<vk::Image, TextureColdData> m_textures {};

        struct RTVColdData
        {
            vk::Format m_format;
            Size16x2 m_size;
        };
        GenerationalPool<vk::ImageView, RTVColdData> m_renderTargetViews {};

        struct RenderPassData
        {
            vk::RenderPass m_renderPass;
            vk::Framebuffer m_framebuffer;
            Size16x2 m_size;
            eastl::vector<vk::ClearValue> m_clearValues;
        };
        GenerationalPool<RenderPassData> m_renderPasses {};

#if !defined(KE_FINAL)
        eastl::shared_ptr<VkDebugHandler> m_debugHandler;
#endif

        // Default constructor and destructor, but moved implementation to cpp for .inl linking
        VkResources();
        ~VkResources();

        [[nodiscard]] GenPool::Handle RegisterTexture(vk::Image _image, Size16x2 _size);

        bool ReleaseTexture(GenPool::Handle _handle, vk::Device _device, bool _free = true);

        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc, vk::Device& _device);

        bool FreeRenderTargetView(GenPool::Handle _handle, vk::Device _device);

        [[nodiscard]] GenPool::Handle CreateRenderPass(const RenderPassDesc& _desc, vk::Device _device);

        bool DestroyRenderPass(GenPool::Handle _handle, vk::Device _device);
    };
} // KryneEngine