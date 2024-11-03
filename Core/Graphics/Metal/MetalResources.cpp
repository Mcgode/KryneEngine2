/**
 * @file
 * @author Max Godefroy
 * @date 30/10/2024.
 */

#include "MetalResources.hpp"

#include <Graphics/Common/Buffer.hpp>
#include <Graphics/Common/ResourceViews/RenderTargetView.hpp>
#include <Graphics/Metal/Helpers/EnumConverters.hpp>
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    MetalResources::MetalResources() = default;
    MetalResources::~MetalResources() = default;

    BufferHandle MetalResources::CreateBuffer(MTL::Device& _device, const BufferCreateDesc& _desc)
    {
        const GenPool::Handle handle = m_buffers.Allocate();

        auto [bufferHot, bufferCold] = m_buffers.GetAll(handle);
        const MTL::ResourceOptions options = MetalConverters::GetResourceStorage(_desc.m_usage);
        bufferHot->m_buffer = _device.newBuffer(_desc.m_desc.m_size, options);
        KE_ASSERT_FATAL(bufferHot->m_buffer != nullptr);
        bufferCold->m_options = options;

        return { handle };
    }

    bool MetalResources::DestroyBuffer(BufferHandle _buffer)
    {
        BufferHotData hotData;
        if (m_buffers.Free(_buffer.m_handle, &hotData))
        {
            hotData.m_buffer.reset();
            return true;
        }
        return false;
    }

    TextureHandle MetalResources::RegisterTexture(MTL::Texture* _texture)
    {
        const GenPool::Handle handle = m_textures.Allocate();
        m_textures.Get(handle)->m_texture = _texture->retain();
        m_textures.Get(handle)->m_isSystemTexture = true;
        return { handle };
    }

    bool MetalResources::UnregisterTexture(TextureHandle _handle)
    {
        return m_textures.Free(_handle.m_handle);
    }

    void MetalResources::UpdateSystemTexture(TextureHandle _handle, MTL::Texture* _texture)
    {
        TextureHotData* textureHotData = m_textures.Get(_handle.m_handle);
        if (textureHotData != nullptr)
        {
            textureHotData->m_texture.reset(_texture->retain());
        }
    }

    RenderTargetViewHandle MetalResources::RegisterRtv(const RenderTargetViewDesc& _desc)
    {
        KE_ERROR("Not fully implemented yet");
        return { GenPool::kInvalidHandle };
    }

    bool MetalResources::UnregisterRtv(RenderTargetViewHandle _handle)
    {
        return m_renderTargetViews.Free(_handle.m_handle);
    }

    RenderTargetViewHandle MetalResources::RegisterRtv(
        const RenderTargetViewDesc& _desc,
        MTL::Texture* _texture)
    {
        VERIFY_OR_RETURN(_desc.m_arrayRangeSize == 1, { GenPool::kInvalidHandle });

        const GenPool::Handle handle = m_renderTargetViews.Allocate();

        auto [rtvHot, rtvCold] = m_renderTargetViews.GetAll(handle);
        rtvHot->m_texture = _texture->retain();
        rtvHot->m_isSystemTexture = _desc.m_texture == GenPool::kInvalidHandle;
        *rtvCold = RtvColdData {
            .m_pixelFormat = _desc.m_format,
            .m_slice = _desc.m_type == TextureTypes::Single3D ? u16(0u) : _desc.m_arrayRangeStart,
            .m_depthSlice = _desc.m_type == TextureTypes::Single3D ? _desc.m_depthStartSlice : u16(0u),
            .m_mipLevel = _desc.m_mipLevel,
            .m_plane = _desc.m_plane,
        };

        return { handle };
    }

    void MetalResources::UpdateSystemTexture(RenderTargetViewHandle _handle, MTL::Texture* _newTexture)
    {
        RtvHotData* rtvHotData = m_renderTargetViews.Get(_handle.m_handle);
        if (rtvHotData != nullptr)
        {
            rtvHotData->m_texture.reset(_newTexture->retain());
        }
    }

    RenderPassHandle MetalResources::CreateRenderPassDescriptor(const RenderPassDesc& _desc)
    {
        const GenPool::Handle handle = m_renderPasses.Allocate();

        auto [hotData, coldData] = m_renderPasses.GetAll(handle);
        hotData->m_descriptor = MTL::RenderPassDescriptor::alloc()->init();

        for (u8 i = 0u; i < _desc.m_colorAttachments.size(); i++)
        {
            MTL::RenderPassColorAttachmentDescriptor* attachment =
                hotData->m_descriptor->colorAttachments()->object(i);
            const RenderPassDesc::Attachment& attachmentDesc = _desc.m_colorAttachments[i];

            RtvHotData* rtvHotData = m_renderTargetViews.Get(attachmentDesc.m_rtv.m_handle);
            MTL::Texture* texture = rtvHotData->m_texture.get();
            KE_ASSERT_FATAL(texture != nullptr);
            if (rtvHotData->m_isSystemTexture)
            {
                hotData->m_systemRtvs.push_back({ attachmentDesc.m_rtv, i });
            }

            attachment->setTexture(texture);
            attachment->setLoadAction(MetalConverters::GetMetalLoadOperation(attachmentDesc.m_loadOperation));
            attachment->setStoreAction(MetalConverters::GetMetalStoreOperation(attachmentDesc.m_storeOperation));
            attachment->setClearColor(MTL::ClearColor(
                attachmentDesc.m_clearColor.r,
                attachmentDesc.m_clearColor.g,
                attachmentDesc.m_clearColor.b,
                attachmentDesc.m_clearColor.a));
        }

        if (_desc.m_depthStencilAttachment.has_value())
        {
            const RenderPassDesc::DepthStencilAttachment& attachmentDesc = _desc.m_depthStencilAttachment.value();

            auto [rtvHot, rtvCold] = m_renderTargetViews.GetAll(attachmentDesc.m_rtv.m_handle);
            KE_ASSERT_FATAL(rtvHot != nullptr);

            if (BitUtils::EnumHasAny(rtvCold->m_plane, TexturePlane::Depth))
            {
                MTL::RenderPassDepthAttachmentDescriptor* attachment = hotData->m_descriptor->depthAttachment();
                attachment->setTexture(rtvHot->m_texture.get());
                attachment->setLoadAction(MetalConverters::GetMetalLoadOperation(attachmentDesc.m_loadOperation));
                attachment->setStoreAction(MetalConverters::GetMetalStoreOperation(attachmentDesc.m_storeOperation));
                attachment->setClearDepth(attachmentDesc.m_clearColor.r);
            }

            if (BitUtils::EnumHasAny(rtvCold->m_plane, TexturePlane::Stencil))
            {
                MTL::RenderPassStencilAttachmentDescriptor* attachment = hotData->m_descriptor->stencilAttachment();
                attachment->setTexture(rtvHot->m_texture.get());
                attachment->setLoadAction(MetalConverters::GetMetalLoadOperation(attachmentDesc.m_stencilLoadOperation));
                attachment->setStoreAction(MetalConverters::GetMetalStoreOperation(attachmentDesc.m_stencilStoreOperation));
                attachment->setClearStencil(attachmentDesc.m_stencilClearValue);
            }
        }

        return { handle };
    }

    bool MetalResources::DestroyRenderPassDescriptor(RenderPassHandle _handle)
    {
        RenderPassHotData data;
        if (m_renderPasses.Free(_handle.m_handle, &data))
        {
            data.m_descriptor.reset();
            return true;
        }
        return false;
    }
} // namespace KryneEngine