/**
 * @file
 * @author Max Godefroy
 * @date 19/02/2024.
 */

#include "VkResources.hpp"
#include "HelperFunctions.hpp"
#include "VkDebugHandler.hpp"
#include "VkDescriptorSetManager.hpp"
#include <Common/Utils/Alignment.hpp>
#include <Graphics/Common/Buffer.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Graphics/Common/RenderPass.hpp>
#include <Graphics/Common/ResourceViews/RenderTargetView.hpp>
#include <Graphics/Common/ResourceViews/ShaderResourceView.hpp>
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

    void VkResources::DestroyAllocator()
    {
        vmaDestroyAllocator(m_allocator);
    }

    BufferHandle VkResources::CreateBuffer(const BufferCreateDesc& _desc, VkDevice _device)
    {
        using namespace VkHelperFunctions;

        const VkBufferCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags = 0,
            .size = _desc.m_desc.m_size,
            .usage = RetrieveBufferUsage(_desc.m_usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VmaAllocationCreateInfo allocationInfo {};
        const MemoryUsage usageType = _desc.m_usage & MemoryUsage::USAGE_TYPE_MASK;
        if (usageType == MemoryUsage::GpuOnly_UsageType)
        {
            allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        }
        else if (usageType == MemoryUsage::StageOnce_UsageType)
        {
            allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        }
        else if (usageType == MemoryUsage::StageEveryFrame_UsageType)
        {
            allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
                                   | VMA_ALLOCATION_CREATE_MAPPED_BIT
                                   | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
        }
        else if (usageType == MemoryUsage::Readback_UsageType)
        {
            allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
                                   | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        const GenPool::Handle handle = m_buffers.Allocate();

        VkBuffer* buffer = m_buffers.Get(handle);
        BufferColdData* coldData = m_buffers.GetCold(handle);
        VkAssert(vmaCreateBuffer(
            m_allocator,
            &createInfo,
            &allocationInfo,
            buffer,
            &coldData->m_allocation,
            &coldData->m_info));


#if !defined(KE_FINAL)
        m_debugHandler->SetName(
            _device,
            VK_OBJECT_TYPE_BUFFER,
            reinterpret_cast<u64>(*buffer),
            _desc.m_desc.m_debugName);
#endif

        return { handle };
    }

    BufferHandle VkResources::CreateStagingBuffer(
        const TextureDesc& _createDesc,
        const eastl::vector<TextureMemoryFootprint>& _footprints,
        VkDevice _device)
    {
        const u64 bufferWidth = _footprints.back().m_offset + _footprints.back().m_lineByteAlignedSize
            * _footprints.back().m_height
            * _footprints.back().m_depth;

        const VkBufferCreateInfo bufferCreateInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags = 0,
            .size = bufferWidth,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        const VmaAllocationCreateInfo allocationCreateInfo {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        };

        const GenPool::Handle handle = m_buffers.Allocate();

        VkBuffer buffer;
        BufferColdData coldData {};
        VkAssert(vmaCreateBuffer(
            m_allocator,
            &bufferCreateInfo,
            &allocationCreateInfo,
            &buffer,
            &coldData.m_allocation,
            &coldData.m_info));

#if !defined(KE_FINAL)
        m_debugHandler->SetName(
            _device,
            VK_OBJECT_TYPE_BUFFER,
            reinterpret_cast<u64>(buffer),
            (_createDesc.m_debugName + "_Staging").c_str());
#endif

        *m_buffers.Get(handle) = buffer;
        *m_buffers.GetCold(handle) = coldData;

        return { handle };
    }

    bool VkResources::DestroyBuffer(BufferHandle _buffer)
    {
        VkBuffer buffer;
        BufferColdData coldData {};

        if (m_buffers.Free(_buffer.m_handle, &buffer, &coldData))
        {
            vmaDestroyBuffer(m_allocator, buffer, coldData.m_allocation);
            return true;
        }

        return false;
    }

    TextureHandle VkResources::RegisterTexture(VkImage _image, const uint3& _dimensions)
    {
        const auto handle = m_textures.Allocate();
        *m_textures.Get(handle) = _image;
        *m_textures.GetCold(handle) = { nullptr, _dimensions };
        return { handle };
    }

    TextureHandle VkResources::CreateTexture(const TextureCreateDesc& _desc, VkDevice _device)
    {
        using namespace VkHelperFunctions;

        const VkImageCreateInfo imageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0,
            .imageType = RetrieveImageType(_desc.m_desc.m_type),
            .format = ToVkFormat(_desc.m_desc.m_format),
            .extent = {
                _desc.m_desc.m_dimensions.x,
                _desc.m_desc.m_dimensions.y,
                _desc.m_desc.m_dimensions.z,
            },
            .mipLevels = _desc.m_desc.m_mipCount,
            .arrayLayers = _desc.m_desc.m_arraySize,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .usage = RetrieveImageUsage(_desc.m_memoryUsage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        const VmaAllocationCreateInfo allocationCreateInfo {
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        };

        VkImage image;
        VmaAllocation allocation;
        VkAssert(vmaCreateImage(
            m_allocator,
            &imageCreateInfo,
            &allocationCreateInfo,
            &image,
            &allocation,
            nullptr));

        const GenPool::Handle handle = m_textures.Allocate();
        *m_textures.Get(handle) = image;
        *m_textures.GetCold(handle) = { allocation, _desc.m_desc.m_dimensions };

#if !defined(KE_FINAL)
        m_debugHandler->SetName(
            _device,
            VK_OBJECT_TYPE_IMAGE,
            reinterpret_cast<u64>(image),
            _desc.m_desc.m_debugName.c_str());
#endif

        return { handle };
    }

    bool VkResources::ReleaseTexture(TextureHandle _texture, VkDevice _device, bool _free)
    {
        VkImage image;
        TextureColdData coldData;
        if (m_textures.Free(_texture.m_handle, &image, &coldData))
        {
            if (_free)
            {
                vmaDestroyImage(m_allocator, image, coldData.m_allocation);
            }
            return true;
        }
        return false;
    }

    TextureSrvHandle VkResources::CreateTextureSrv(const TextureSrvDesc& _srvDesc, VkDevice _device)
    {
        auto* image = m_textures.Get(_srvDesc.m_texture.m_handle);
        if (image == nullptr)
        {
            return { GenPool::kInvalidHandle };
        }

        const GenPool::Handle srvHandle = m_imageViews.Allocate();

        VkImageView imageView = CreateImageView(
            _device,
            *image,
            VkHelperFunctions::RetrieveImageViewType(_srvDesc.m_viewType),
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

        return { srvHandle };
    }

    bool VkResources::DestroyTextureSrv(TextureSrvHandle _textureSrv, VkDevice _device)
    {
        VkImageView imageView;

        if (m_imageViews.Free(_textureSrv.m_handle, &imageView))
        {
            vkDestroyImageView(_device, imageView, nullptr);
            return true;
        }
        return false;
    }

    SamplerHandle VkResources::CreateSampler(const SamplerDesc& _samplerDesc, VkDevice _device)
    {
        VkSamplerCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = _samplerDesc.m_magFilter == SamplerDesc::Filter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST,
            .minFilter = _samplerDesc.m_minFilter == SamplerDesc::Filter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST,
            .mipmapMode = _samplerDesc.m_mipFilter == SamplerDesc::Filter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VkHelperFunctions::ToVkAddressMode(_samplerDesc.m_addressModeU),
            .addressModeV = VkHelperFunctions::ToVkAddressMode(_samplerDesc.m_addressModeV),
            .addressModeW = VkHelperFunctions::ToVkAddressMode(_samplerDesc.m_addressModeW),
            .mipLodBias = _samplerDesc.m_lodBias,
            .anisotropyEnable = _samplerDesc.m_anisotropy > 0,
            .maxAnisotropy = static_cast<float>(_samplerDesc.m_anisotropy),
            .compareEnable = _samplerDesc.m_opType != SamplerDesc::OpType::Blend,
            .minLod = _samplerDesc.m_lodMin,
            .maxLod = _samplerDesc.m_lodMax,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            .unnormalizedCoordinates = false,
        };

        switch (_samplerDesc.m_opType)
        {
        case SamplerDesc::OpType::Blend:
            createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            break;
        case SamplerDesc::OpType::Minimum:
            createInfo.compareOp = VK_COMPARE_OP_LESS;
            break;
        case SamplerDesc::OpType::Maximum:
            createInfo.compareOp = VK_COMPARE_OP_GREATER;
            break;
        }

        const GenPool::Handle handle = m_samplers.Allocate();
        VkAssert(vkCreateSampler(_device, &createInfo, nullptr, m_samplers.Get(handle)));
        return { handle };
    }

    bool VkResources::DestroySampler(SamplerHandle _sampler, VkDevice _device)
    {
        VkSampler sampler;
        if (m_samplers.Free(_sampler.m_handle, &sampler))
        {
            vkDestroySampler(_device, sampler, nullptr);
            return true;
        }
        return false;
    }

    RenderTargetViewHandle VkResources::CreateRenderTargetView(
            const RenderTargetViewDesc &_desc,
            VkDevice &_device)
    {
        auto* image = m_textures.Get(_desc.m_texture.m_handle);
        if (image == nullptr)
        {
            return { GenPool::kInvalidHandle };
        }

        const auto rtvHandle = m_renderTargetViews.Allocate();

        const VkFormat format = VkHelperFunctions::ToVkFormat(_desc.m_format);

        VkImageView imageView = CreateImageView(
            _device,
            *image,
            VkHelperFunctions::RetrieveImageViewType(_desc.m_type),
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
            {
                .m_width = (u16) m_textures.GetCold(_desc.m_texture.m_handle)->m_dimensions.x,
                .m_height = (u16) m_textures.GetCold(_desc.m_texture.m_handle)->m_dimensions.y,
            }
        };

        return { rtvHandle };
    }

    bool VkResources::FreeRenderTargetView(RenderTargetViewHandle _rtv, VkDevice _device)
    {
        VkImageView imageView;
        if (m_renderTargetViews.Free(_rtv.m_handle, &imageView))
        {
            vkDestroyImageView(_device, imageView, nullptr);
            return true;
        }
        return false;
    }

    RenderPassHandle VkResources::CreateRenderPass(const RenderPassDesc &_desc, VkDevice _device)
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
            auto* rtvColdData = m_renderTargetViews.GetCold(attachment.m_rtv.m_handle);
            VERIFY_OR_RETURN(rtvColdData != nullptr, { GenPool::kInvalidHandle });

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
            rtvs.push_back(*m_renderTargetViews.Get(attachment.m_rtv.m_handle));
            attachmentReferences.push_back(ref);

            VkClearColorValue clearColorValue;
            memcpy(&clearColorValue.float32, &attachment.m_clearColor[0], sizeof(clearColorValue.float32));
            clearValues.push_back({ .color = clearColorValue });
        }

        VkAttachmentReference depthRef;
        if (_desc.m_depthStencilAttachment.has_value())
        {
            const auto& attachment = _desc.m_depthStencilAttachment.value();

            auto* rtvColdData = m_renderTargetViews.GetCold(attachment.m_rtv.m_handle);
            VERIFY_OR_RETURN(rtvColdData != nullptr, { GenPool::kInvalidHandle });

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
            rtvs.push_back(*m_renderTargetViews.Get(attachment.m_rtv.m_handle));
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

        return { handle };
    }

    bool VkResources::DestroyRenderPass(RenderPassHandle _renderPass, VkDevice _device)
    {
        RenderPassData data;
        if (!m_renderPasses.Free(_renderPass.m_handle, &data))
        {
            return false;
        }

        data.m_clearValues.clear();
        vkDestroyFramebuffer(_device, data.m_framebuffer, nullptr);
        vkDestroyRenderPass(_device, data.m_renderPass, nullptr);

        return true;
    }

    ShaderModuleHandle VkResources::CreateShaderModule(void* _bytecodeData, u64 _bytecodeSize, VkDevice _device)
    {
        VERIFY_OR_RETURN(Alignment::IsAligned<u64>(_bytecodeSize, 4u), { GenPool::kInvalidHandle });

        const VkShaderModuleCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = _bytecodeSize,
            .pCode = static_cast<u32*>(_bytecodeData),
        };

        const GenPool::Handle handle = m_shaderModules.Allocate();
        VkAssert(vkCreateShaderModule(_device, &createInfo, nullptr, m_shaderModules.Get(handle)));
        return { handle };
    }

    bool VkResources::DestroyShaderModule(ShaderModuleHandle _shaderModule, VkDevice _device)
    {
        VkShaderModule module;
        if (m_shaderModules.Free(_shaderModule.m_handle, &module))
        {
            vkDestroyShaderModule(_device, module, nullptr);
            return true;
        }
        return false;
    }

    PipelineLayoutHandle VkResources::CreatePipelineLayout(
        const PipelineLayoutDesc& _desc, VkDevice _device, VkDescriptorSetManager* _setManager)
    {
        DynamicArray<VkDescriptorSetLayout> setLayouts(_desc.m_descriptorSets.size());
        for (auto i = 0u; i < setLayouts.Size(); i++)
        {
            setLayouts[i] = _setManager->GetDescriptorSetLayout(_desc.m_descriptorSets[i]);
        }

        DynamicArray<VkPushConstantRange> pushConstants(_desc.m_pushConstants.size());
        for (auto i = 0u; i < pushConstants.Size(); i++)
        {
            const PushConstantDesc& pushConstant = _desc.m_pushConstants[i];
            pushConstants[i] = {
                .stageFlags = VkHelperFunctions::ToVkShaderStageFlags(pushConstant.m_visibility),
                .offset = pushConstant.m_offset,
                .size = pushConstant.m_sizeInBytes,
            };
        }

        const VkPipelineLayoutCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<u32>(setLayouts.Size()),
            .pSetLayouts = setLayouts.Data(),
            .pushConstantRangeCount = static_cast<u32>(pushConstants.Size()),
            .pPushConstantRanges = pushConstants.Data(),
        };

        const GenPool::Handle handle = m_pipelineLayouts.Allocate();
        VkAssert(vkCreatePipelineLayout(
            _device,
            &createInfo,
            nullptr,
            m_pipelineLayouts.Get(handle)));

        return { handle };
    }

    bool VkResources::DestroyPipelineLayout(PipelineLayoutHandle _pipeline, VkDevice _device)
    {
        VkPipelineLayout layout;
        if (m_pipelineLayouts.Free(_pipeline.m_handle, &layout))
        {
            vkDestroyPipelineLayout(_device, layout, nullptr);
            return true;
        }
        return false;
    }

    GraphicsPipelineHandle VkResources::CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc, VkDevice _device)
    {
        KE_ERROR("Not yet implemented");
        return { GenPool::kInvalidHandle };
    }

    bool VkResources::DestroyGraphicsPipeline(GraphicsPipelineHandle _pipeline, VkDevice _device)
    {
        VkPipeline pipeline;
        if (m_pipelines.Free(_pipeline.m_handle, &pipeline))
        {
            vkDestroyPipeline(_device, pipeline, nullptr);
            return true;
        }
        return false;
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
