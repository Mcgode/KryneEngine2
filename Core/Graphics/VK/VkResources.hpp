/**
 * @file
 * @author Max Godefroy
 * @date 19/02/2024.
 */

#pragma once

#include "VkHeaders.hpp"
#include <EASTL/shared_ptr.h>
#include <Graphics/Common/Texture.hpp>
#include <vk_mem_alloc.h>

namespace KryneEngine
{
    namespace GraphicsCommon
    {
        struct ApplicationInfo;
    }

    struct BufferCreateDesc;
    struct RenderTargetViewDesc;
    struct RenderPassDesc;
    struct TextureSrvDesc;

    class VkDebugHandler;

    struct VkResources
    {
        friend class VkGraphicsContext;

        struct BufferColdData
        {
            VmaAllocation m_allocation;
            VmaAllocationInfo m_info;
        };
        GenerationalPool<VkBuffer, BufferColdData> m_buffers {};

        struct TextureColdData
        {
            VmaAllocation m_allocation;
            uint3 m_dimensions;
        };
        GenerationalPool<VkImage, TextureColdData> m_textures {};

        GenerationalPool<VkImageView> m_imageViews {};

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

        void InitAllocator(
            const GraphicsCommon::ApplicationInfo& _appInfo,
            VkDevice _device,
            VkPhysicalDevice _physicalDevice,
            VkInstance _instance);

        void DestroyAllocator();

        [[nodiscard]] GenPool::Handle CreateBuffer(const BufferCreateDesc& _desc, VkDevice _device);

        [[nodiscard]] GenPool::Handle CreateStagingBuffer(
            const TextureDesc& _createDesc,
            const eastl::vector<TextureMemoryFootprint>& _footprints,
            VkDevice _device);

        bool DestroyBuffer(GenPool::Handle _bufferHandle);

        [[nodiscard]] GenPool::Handle RegisterTexture(VkImage _image, const uint3& _dimensions);

        [[nodiscard]] GenPool::Handle CreateTexture(const TextureCreateDesc& _desc, VkDevice _device);

        bool ReleaseTexture(GenPool::Handle _handle, VkDevice _device, bool _free = true);

        [[nodiscard]] GenPool::Handle CreateTextureSrv(const TextureSrvDesc& _srvDesc, VkDevice _device);

        bool DestroyTextureSrv(GenPool::Handle _handle, VkDevice _device);

        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc, VkDevice& _device);

        bool FreeRenderTargetView(GenPool::Handle _handle, VkDevice _device);

        [[nodiscard]] GenPool::Handle CreateRenderPass(const RenderPassDesc& _desc, VkDevice _device);

        bool DestroyRenderPass(GenPool::Handle _handle, VkDevice _device);

    private:
        VmaAllocator m_allocator;

        [[nodiscard]] VkImageView CreateImageView(
            VkDevice _device,
            VkImage _image,
            VkImageViewType _viewType,
            VkFormat _format,
            VkComponentMapping _componentMapping,
            VkImageAspectFlags _aspectFlags,
            u32 _mipStart,
            u32 _mipCount,
            u32 _arrayStart,
            u32 _arraySize);
    };
} // KryneEngine