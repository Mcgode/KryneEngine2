/**
 * @file
 * @author Max Godefroy
 * @date 19/02/2024.
 */

#include "VkResources.hpp"
#include "VkDebugHandler.hpp"
#include "HelperFunctions.hpp"
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Graphics/Common/ResourceViews/RenderTargetView.hpp>
#include <Graphics/Common/ResourceViews/ShaderResourceView.hpp>
#include <Graphics/Common/RenderPass.hpp>
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    VkResources::VkResources()  = default;
    VkResources::~VkResources() = default;

    void VkResources::InitAllocator(
        const GraphicsCommon::ApplicationInfo& _appInfo,
        VkDevice _device,
        VkPhysicalDevice _physicalDevice,
        VkInstance _instance)
    {
        const VmaAllocatorCreateInfo createInfo {
            .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
            .physicalDevice = _physicalDevice,
            .device = _device,
            .instance = _instance,
            .vulkanApiVersion = VkHelperFunctions::GetApiVersion(_appInfo.m_api),
        };

        vmaCreateAllocator(&createInfo, &m_allocator);
    }

    GenPool::Handle VkResources::RegisterTexture(VkImage _image, Size16x2 _size)
    {
        const auto handle = m_textures.Allocate();
        *m_textures.Get(handle) = _image;
        *m_textures.GetCold(handle) = { _size };
        return handle;
    }

    bool VkResources::ReleaseTexture(GenPool::Handle _handle, VkDevice _device, bool _free)
    {
        VkImage image;
        if (m_textures.Free(_handle, &image))
        {
            if (_free)
            {
                vkDestroyImage(_device, image, nullptr);
            }
            return true;
        }
        return false;
    }

    GenPool::Handle VkResources::CreateTextureSrv(const TextureSrvDesc& _srvDesc, VkDevice _device)
    {
        auto* image = m_textures.Get(_srvDesc.m_textureHandle);
        if (image == nullptr)
        {
            return GenPool::kInvalidHandle;
        }

        const GenPool::Handle srvHandle = m_imageViews.Allocate();

        VkImageView imageView = CreateImageView(
            _device,
            *image,
            VkHelperFunctions::RetrieveViewType(_srvDesc.m_viewType),
            VkHelperFunctions::ToVkFormat(_srvDesc.m_format),
            VkHelperFunctions::ToVkComponentMapping(_srvDesc.m_componentsMapping),
            VK_IMAGE_ASPECT_COLOR_BIT,
            _srvDesc.m_minMip,
            _srvDesc.m_maxMip - _srvDesc.m_minMip + 1,
            _srvDesc.m_arrayStart,
            _srvDesc.m_arrayRange);

#if !defined(KE_FINAL)
        {
            const u64 handle = reinterpret_cast<u64>(imageView);
            m_debugHandler->SetName(_device, VK_OBJECT_TYPE_IMAGE_VIEW, handle, _srvDesc.m_debugName.c_str());
        }
#endif

        *m_imageViews.Get(srvHandle) = imageView;

        return srvHandle;
    }

    GenPool::Handle VkResources::CreateRenderTargetView(
            const RenderTargetViewDesc &_desc,
            VkDevice &_device)
    {
        auto* image = m_textures.Get(_desc.m_textureHandle);
        if (image == nullptr)
        {
            return GenPool::kInvalidHandle;
        }

        const auto rtvHandle = m_renderTargetViews.Allocate();

        const VkFormat format = VkHelperFunctions::ToVkFormat(_desc.m_format);

        VkImageView imageView = CreateImageView(
            _device,
            *image,
            VkHelperFunctions::RetrieveViewType(_desc.m_type),
            format,
            { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,  VK_COMPONENT_SWIZZLE_IDENTITY },
            VkHelperFunctions::RetrieveAspectMask(_desc.m_plane),
            _desc.m_mipLevel,
            1,
            _desc.m_arrayRangeStart,
            _desc.m_arrayRangeSize);

#if !defined(KE_FINAL)
        {
            const u64 handle = (u64)static_cast<VkImageView>(imageView);
            m_debugHandler->SetName(_device, VK_OBJECT_TYPE_IMAGE_VIEW, handle, _desc.m_debugName.data());
        }
#endif

        *m_renderTargetViews.Get(rtvHandle) = imageView;
        *m_renderTargetViews.GetCold(rtvHandle) = {
                format,
                m_textures.GetCold(_desc.m_textureHandle)->m_size
        };

        return rtvHandle;
    }

    bool VkResources::FreeRenderTargetView(GenPool::Handle _handle, VkDevice _device)
    {
        VkImageView imageView;
        if (m_renderTargetViews.Free(_handle, &imageView))
        {
            vkDestroyImageView(_device, imageView, nullptr);
            return true;
        }
        return false;
    }

    GenPool::Handle VkResources::CreateRenderPass(const RenderPassDesc &_desc, VkDevice _device)
    {
        constexpr auto convertLoadOp = [](RenderPassDesc::Attachment::LoadOperation _op)
        {
            switch (_op)
            {
                case RenderPassDesc::Attachment::LoadOperation::Load:
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
                case RenderPassDesc::Attachment::LoadOperation::Clear:
                    return VK_ATTACHMENT_LOAD_OP_CLEAR;
                case RenderPassDesc::Attachment::LoadOperation::DontCare:
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            }
            KE_ERROR("Unreachable code");
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        };
        constexpr auto convertStoreOp = [](RenderPassDesc::Attachment::StoreOperation _op)
        {
            switch (_op)
            {
                case RenderPassDesc::Attachment::StoreOperation::Store:
                case RenderPassDesc::Attachment::StoreOperation::Resolve:
                    return VK_ATTACHMENT_STORE_OP_STORE;
                case RenderPassDesc::Attachment::StoreOperation::DontCare:
                    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }
            KE_ERROR("Unreachable code");
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        };

        eastl::fixed_vector<
                VkAttachmentDescription,
                RenderPassDesc::kMaxSupportedColorAttachments + 1, // Include optional depth attachment
                false> attachments {};
        eastl::fixed_vector<
                VkImageView,
                RenderPassDesc::kMaxSupportedColorAttachments + 1, // Include optional depth attachment
                false> rtvs {};
        eastl::fixed_vector<
                VkAttachmentReference,
                RenderPassDesc::kMaxSupportedColorAttachments,
                false> attachmentReferences {};
        eastl::vector<VkClearValue> clearValues;

        Size16x2 size {};

        for (const auto& attachment: _desc.m_colorAttachments)
        {
            auto* rtvColdData = m_renderTargetViews.GetCold(attachment.m_rtv);
            VERIFY_OR_RETURN(rtvColdData != nullptr, GenPool::kInvalidHandle);

            if (size.m_width == 0)
            {
                size = rtvColdData->m_size;
            }
            else
            {
                KE_ASSERT(size.m_width == rtvColdData->m_size.m_width && size.m_height == rtvColdData->m_size.m_height);
            }

            VkAttachmentDescription desc {
                .flags = 0,
                .format = rtvColdData->m_format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = convertLoadOp(attachment.m_loadOperation),
                .storeOp = convertStoreOp(attachment.m_storeOperation),
                .initialLayout = VkHelperFunctions::ToVkLayout(attachment.m_initialLayout),
                .finalLayout = VkHelperFunctions::ToVkLayout(attachment.m_finalLayout),
            };

            VkAttachmentReference ref {
                u32(attachments.size()),
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            };

            attachments.push_back(desc);
            rtvs.push_back(*m_renderTargetViews.Get(attachment.m_rtv));
            attachmentReferences.push_back(ref);

            VkClearColorValue clearColorValue;
            memcpy(&clearColorValue.float32, &attachment.m_clearColor[0], sizeof(clearColorValue.float32));
            clearValues.push_back({ .color = clearColorValue });
        }

        VkAttachmentReference depthRef;
        if (_desc.m_depthStencilAttachment.has_value())
        {
            const auto& attachment = _desc.m_depthStencilAttachment.value();

            auto* rtvColdData = m_renderTargetViews.GetCold(attachment.m_rtv);
            VERIFY_OR_RETURN(rtvColdData != nullptr, GenPool::kInvalidHandle);

            if (size.m_width == 0)
            {
                size = rtvColdData->m_size;
            }
            else
            {
                KE_ASSERT(size.m_width == rtvColdData->m_size.m_width && size.m_height == rtvColdData->m_size.m_height);
            }

            VkAttachmentDescription desc {
                .flags = 0,
                .format = rtvColdData->m_format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = convertLoadOp(attachment.m_loadOperation),
                .storeOp = convertStoreOp(attachment.m_storeOperation),
                .stencilLoadOp = convertLoadOp(attachment.m_loadOperation),
                .initialLayout = VkHelperFunctions::ToVkLayout(attachment.m_initialLayout),
                .finalLayout = VkHelperFunctions::ToVkLayout(attachment.m_finalLayout)
            };

            depthRef = {
                    u32(attachments.size()),
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            };

            attachments.push_back(desc);
            rtvs.push_back(*m_renderTargetViews.Get(attachment.m_rtv));
            clearValues.push_back(VkClearValue {
                .depthStencil = {
                    .depth = attachment.m_clearColor.r,
                    .stencil = attachment.m_stencilClearValue
                }
            });
        }

        VkSubpassDescription subpassDescription {
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = static_cast<u32>(attachmentReferences.size()),
                .pColorAttachments = attachmentReferences.data(),
                .pDepthStencilAttachment = _desc.m_depthStencilAttachment.has_value() ? &depthRef : nullptr
        };

        VkRenderPassCreateInfo createInfo {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .flags = 0,
                .attachmentCount = static_cast<u32>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = 1,
                .pSubpasses = &subpassDescription
        };

        const auto handle = m_renderPasses.Allocate();
        auto& renderPassData = *m_renderPasses.Get(handle);
        VkAssert(vkCreateRenderPass(_device, &createInfo, nullptr, &renderPassData.m_renderPass));
        renderPassData.m_size = size;
        renderPassData.m_clearValues = eastl::move(clearValues);

        VkFramebufferCreateInfo framebufferCreateInfo {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .flags = 0,
                .renderPass = renderPassData.m_renderPass,
                .attachmentCount = static_cast<u32>(rtvs.size()),
                .pAttachments = rtvs.data(),
                .width = size.m_width,
                .height = size.m_height,
                .layers = 1
        };
        VkAssert(vkCreateFramebuffer(_device, &framebufferCreateInfo, nullptr, &renderPassData.m_framebuffer));

#if !defined(KE_FINAL)
        {
            const u64 objectHandle = (u64)static_cast<VkRenderPass>(renderPassData.m_renderPass);
            const eastl::string name = _desc.m_debugName + "/RenderPass";
            m_debugHandler->SetName(_device, VK_OBJECT_TYPE_RENDER_PASS, objectHandle, name.c_str());
        }
        {
            const u64 objectHandle = (u64)static_cast<VkFramebuffer>(renderPassData.m_framebuffer);
            const eastl::string name = _desc.m_debugName + "/Framebuffer";
            m_debugHandler->SetName(_device, VK_OBJECT_TYPE_FRAMEBUFFER, objectHandle, name.c_str());
        }
#endif

        return handle;
    }

    bool VkResources::DestroyRenderPass(GenPool::Handle _handle, VkDevice _device)
    {
        RenderPassData data;
        if (!m_renderPasses.Free(_handle, &data))
        {
            return false;
        }

        data.m_clearValues.clear();
        vkDestroyFramebuffer(_device, data.m_framebuffer, nullptr);
        vkDestroyRenderPass(_device, data.m_renderPass, nullptr);

        return true;
    }

    VkImageView VkResources::CreateImageView(
        VkDevice _device,
        VkImage _image,
        VkImageViewType _viewType,
        VkFormat _format,
        VkComponentMapping _componentMapping,
        VkImageAspectFlags _aspectFlags,
        u32 _mipStart,
        u32 _mipCount,
        u32 _arrayStart,
        u32 _arraySize)
    {
        VkImageViewCreateInfo imageViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .flags = 0,
            .image = _image,
            .viewType = _viewType,
            .format = _format,
            .components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,  VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = {
                .aspectMask = _aspectFlags,
                .baseMipLevel = _mipStart,
                .levelCount = _mipCount,
                .baseArrayLayer = _arrayStart,
                .layerCount = _arraySize,
            }
        };

        VkImageView imageView = nullptr;
        VkAssert(vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &imageView));

        return imageView;
    }


}
