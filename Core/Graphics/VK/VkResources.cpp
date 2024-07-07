/**
 * @file
 * @author Max Godefroy
 * @date 19/02/2024.
 */

#include "VkResources.hpp"
#include "HelperFunctions.hpp"
#include <Graphics/Common/RenderTargetView.hpp>
#include <Graphics/Common/RenderPass.hpp>
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    VkResources::VkResources()  = default;
    VkResources::~VkResources() = default;

    GenPool::Handle VkResources::RegisterTexture(vk::Image _image, Size16x2 _size)
    {
        const auto handle = m_textures.Allocate();
        *m_textures.Get(handle) = _image;
        *m_textures.GetCold(handle) = { _size };
        return handle;
    }

    bool VkResources::ReleaseTexture(GenPool::Handle _handle, vk::Device _device, bool _free)
    {
        vk::Image image;
        if (m_textures.Free(_handle, &image))
        {
            if (_free)
            {
                _device.destroy(image);
            }
            return true;
        }
        return false;
    }

    GenPool::Handle VkResources::CreateRenderTargetView(
            const RenderTargetViewDesc &_desc,
            vk::Device &_device)
    {
        auto* image = m_textures.Get(_desc.m_textureHandle);
        if (image == nullptr)
        {
            return GenPool::kInvalidHandle;
        }

        const auto rtvHandle = m_renderTargetViews.Allocate();

        vk::ImageViewCreateInfo imageViewCreateInfo(
                {},
                *image,
                VkHelperFunctions::RetrieveViewType(_desc.m_type),
                VkHelperFunctions::ToVkFormat(_desc.m_format),
                {},
                {
                        VkHelperFunctions::RetrieveAspectMask(_desc.m_plane),
                        _desc.m_mipLevel,
                        1,
                        _desc.m_arrayRangeStart,
                        _desc.m_arrayRangeSize,
                }
        );

        *m_renderTargetViews.Get(rtvHandle) = _device.createImageView(imageViewCreateInfo);
        *m_renderTargetViews.GetCold(rtvHandle) = {
                imageViewCreateInfo.format,
                m_textures.GetCold(_desc.m_textureHandle)->m_size
        };

        return rtvHandle;
    }

    bool VkResources::FreeRenderTargetView(GenPool::Handle _handle, vk::Device _device)
    {
        vk::ImageView imageView;
        if (m_renderTargetViews.Free(_handle, &imageView))
        {
            _device.destroy(imageView);
            return true;
        }
        return false;
    }

    GenPool::Handle VkResources::CreateRenderPass(const RenderPassDesc &_desc, vk::Device _device)
    {
        constexpr auto convertLoadOp = [](RenderPassDesc::Attachment::LoadOperation _op)
        {
            switch (_op)
            {
                case RenderPassDesc::Attachment::LoadOperation::Load:
                    return vk::AttachmentLoadOp::eLoad;
                case RenderPassDesc::Attachment::LoadOperation::Clear:
                    return vk::AttachmentLoadOp::eClear;
                case RenderPassDesc::Attachment::LoadOperation::DontCare:
                    return vk::AttachmentLoadOp::eDontCare;
            }
            KE_ERROR("Unreachable code");
            return vk::AttachmentLoadOp::eDontCare;
        };
        constexpr auto convertStoreOp = [](RenderPassDesc::Attachment::StoreOperation _op)
        {
            switch (_op)
            {
                case RenderPassDesc::Attachment::StoreOperation::Store:
                case RenderPassDesc::Attachment::StoreOperation::Resolve:
                    return vk::AttachmentStoreOp::eStore;
                case RenderPassDesc::Attachment::StoreOperation::DontCare:
                    return vk::AttachmentStoreOp::eDontCare;
            }
            KE_ERROR("Unreachable code");
            return vk::AttachmentStoreOp::eDontCare;
        };

        eastl::fixed_vector<
                vk::AttachmentDescription,
                RenderPassDesc::kMaxSupportedColorAttachments + 1, // Include optional depth attachment
                false> attachments {};
        eastl::fixed_vector<
                vk::ImageView,
                RenderPassDesc::kMaxSupportedColorAttachments + 1, // Include optional depth attachment
                false> rtvs {};
        eastl::fixed_vector<
                vk::AttachmentReference,
                RenderPassDesc::kMaxSupportedColorAttachments,
                false> attachmentReferences {};
        eastl::vector<vk::ClearValue> clearValues;

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

            vk::AttachmentDescription desc;
            desc.format = rtvColdData->m_format;
            desc.loadOp = convertLoadOp(attachment.m_loadOperation);
            desc.storeOp = convertStoreOp(attachment.m_storeOperation);
            desc.initialLayout = VkHelperFunctions::ToVkLayout(attachment.m_initialLayout);
            desc.finalLayout = VkHelperFunctions::ToVkLayout(attachment.m_finalLayout);

            vk::AttachmentReference ref {
                u32(attachments.size()),
                vk::ImageLayout::eColorAttachmentOptimal
            };

            attachments.push_back(desc);
            rtvs.push_back(*m_renderTargetViews.Get(attachment.m_rtv));
            attachmentReferences.push_back(ref);

            vk::ClearColorValue clearColorValue;
            memcpy(&clearColorValue.float32, &attachment.m_clearColor[0], sizeof(clearColorValue.float32));
            clearValues.push_back(clearColorValue);
        }

        vk::AttachmentReference depthRef;
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

            vk::AttachmentDescription desc;
            desc.format = rtvColdData->m_format;
            desc.loadOp = convertLoadOp(attachment.m_loadOperation);
            desc.storeOp = convertStoreOp(attachment.m_storeOperation);
            desc.stencilLoadOp = convertLoadOp(attachment.m_loadOperation);
            desc.initialLayout = VkHelperFunctions::ToVkLayout(attachment.m_initialLayout);
            desc.finalLayout = VkHelperFunctions::ToVkLayout(attachment.m_finalLayout);

            depthRef = {
                    u32(attachments.size()),
                    vk::ImageLayout::eDepthStencilAttachmentOptimal
            };

            attachments.push_back(desc);
            rtvs.push_back(*m_renderTargetViews.Get(attachment.m_rtv));
            clearValues.push_back(vk::ClearDepthStencilValue{
                attachment.m_clearColor.r,
                attachment.m_stencilClearValue
            });
        }

        vk::SubpassDescription subpassDescription;
        subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDescription.colorAttachmentCount = attachmentReferences.size();
        subpassDescription.pColorAttachments = attachmentReferences.data();
        subpassDescription.pDepthStencilAttachment = _desc.m_depthStencilAttachment.has_value() ? &depthRef : nullptr;

        vk::RenderPassCreateInfo createInfo {
                {},
                u32(attachments.size()),
                attachments.data(),
                1,
                &subpassDescription
        };

        const auto handle = m_renderPasses.Allocate();
        auto& renderPassData = *m_renderPasses.Get(handle);
        renderPassData.m_renderPass = _device.createRenderPass(createInfo);
        renderPassData.m_size = size;
        renderPassData.m_clearValues = eastl::move(clearValues);

        vk::FramebufferCreateInfo framebufferCreateInfo {
                {},
                renderPassData.m_renderPass,
                u32(rtvs.size()),
                rtvs.data(),
                size.m_width,
                size.m_height,
                1
        };
        renderPassData.m_framebuffer = _device.createFramebuffer(framebufferCreateInfo);

        return handle;
    }

    bool VkResources::DestroyRenderPass(GenPool::Handle _handle, vk::Device _device)
    {
        RenderPassData data;
        if (!m_renderPasses.Free(_handle, &data))
        {
            return false;
        }

        data.m_clearValues.clear();
        _device.destroy(data.m_framebuffer);
        _device.destroy(data.m_renderPass);

        return true;
    }


}
