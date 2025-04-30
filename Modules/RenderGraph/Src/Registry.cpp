/**
 * @file
 * @author Max Godefroy
 * @date 14/11/2024.
 */

#include "KryneEngine/Modules/RenderGraph/Registry.hpp"

#include "KryneEngine/Core/Graphics/GraphicsContext.hpp"
#include <KryneEngine/Core/Graphics/ResourceViews/RenderTargetView.hpp>
#include <KryneEngine/Core/Graphics/ResourceViews/ShaderResourceView.hpp>
#include <KryneEngine/Core/Memory/SimplePool.inl>

#include "KryneEngine/Modules/RenderGraph/Descriptors/RenderTargetViewDesc.hpp"
#include "KryneEngine/Modules/RenderGraph/Resource.hpp"

namespace KryneEngine::Modules::RenderGraph
{
    Registry::Registry() = default;
    Registry::~Registry() = default;

    SimplePoolHandle Registry::RegisterRawTexture(TextureHandle _texture, const eastl::string_view& _name)
    {
        const SimplePoolHandle handle = m_resources.AllocateAndInit(Resource {
            .m_type = ResourceType::RawTexture,
            .m_owned = false,
            .m_rawTextureData = {
                .m_texture = _texture,
            },
#if !defined(KE_FINAL)
            .m_name = _name.data(),
#endif
        });
        return handle;
    }

    SimplePoolHandle Registry::RegisterRawBuffer(BufferHandle _buffer, const eastl::string_view& _name)
    {
        const SimplePoolHandle handle = m_resources.AllocateAndInit(Resource {
            .m_type = ResourceType::RawBuffer,
            .m_owned = false,
            .m_rawBufferData = {
                .m_buffer = _buffer,
            },
#if !defined(KE_FINAL)
            .m_name = _name.data(),
#endif
        });
        return handle;
    }

    SimplePoolHandle Registry::RegisterTextureSrv(
        TextureSrvHandle _textureSrv,
        SimplePoolHandle _textureResource,
        const eastl::string_view& _name)
    {
        KE_ASSERT(m_resources.Get(_textureResource).m_type == ResourceType::RawTexture);

        // Add ref to underlying texture resource
        m_resources.AddRef(_textureResource);

        const SimplePoolHandle handle = m_resources.AllocateAndInit(Resource {
            .m_type = ResourceType::TextureSrv,
            .m_owned = false,
            .m_textureSrvData = {
                .m_textureSrv = _textureSrv,
                .m_textureResource = _textureResource,
            },
#if !defined(KE_FINAL)
            .m_name = _name.data(),
#endif
        });
        return handle;
    }

    SimplePoolHandle Registry::RegisterCbv(
        BufferCbvHandle _cbv,
        SimplePoolHandle _bufferResource,
        const eastl::string_view& _name)
    {
        KE_ASSERT(m_resources.Get(_bufferResource).m_type == ResourceType::RawBuffer);

        m_resources.AddRef(_bufferResource);
        const SimplePoolHandle handle = m_resources.AllocateAndInit(Resource {
            .m_type = ResourceType::BufferCbv,
            .m_owned = false,
            .m_bufferCbvData = {
                .m_cbv = _cbv,
                .m_bufferResource = _bufferResource,
            }
        });
        return handle;
    }

    SimplePoolHandle Registry::RegisterRenderTargetView(
        RenderTargetViewHandle _rtv,
        SimplePoolHandle _textureResource,
        const eastl::string_view& _name)
    {

        KE_ASSERT(m_resources.Get(_textureResource).m_type == ResourceType::RawTexture);

        // Add ref to underlying texture resource
        m_resources.AddRef(_textureResource);

        const SimplePoolHandle handle = m_resources.AllocateAndInit(Resource {
            .m_type = ResourceType::RenderTargetView,
            .m_owned = false,
            .m_renderTargetViewData = {
                .m_renderTargetView = _rtv,
                .m_textureResource = _textureResource,
            },
#if !defined(KE_FINAL)
            .m_name = _name.data(),
#endif
        });
        return handle;
    }

    SimplePoolHandle Registry::CreateRawTexture(
        GraphicsContext* _graphicsContext,
        const TextureCreateDesc& _desc)
    {
        TextureHandle texture = _graphicsContext->CreateTexture(_desc);
        return m_resources.AllocateAndInit(Resource {
            .m_type = ResourceType::RawTexture,
            .m_owned = true,
            .m_rawTextureData = {
                .m_texture = texture,
            },
#if !defined(KE_FINAL)
            .m_name = _desc.m_desc.m_debugName.data(),
#endif
        });
    }

    SimplePoolHandle Registry::CreateRenderTargetView(
        GraphicsContext* _graphicsContext,
        const RenderTargetViewDesc& _desc,
        eastl::string_view _name)
    {
        Resource& resource = m_resources.Get(_desc.m_textureResource);
        VERIFY_OR_RETURN(resource.m_type == ResourceType::RawTexture, ~0ull);

        const KryneEngine::RenderTargetViewDesc desc {
            .m_texture = resource.m_rawTextureData.m_texture,
            .m_format = _desc.m_format,
            .m_type = _desc.m_type,
            .m_plane = _desc.m_plane,
            .m_arrayRangeStart = _desc.m_arrayRangeStart,
            .m_arrayRangeSize = _desc.m_arrayRangeSize,
            .m_mipLevel = _desc.m_mipLevel,
#if !defined(KE_FINAL)
            .m_debugName = _name.data(),
#endif
        };

        return m_resources.AllocateAndInit(Resource {
            .m_type = ResourceType::RenderTargetView,
            .m_owned = true,
            .m_renderTargetViewData = {
                .m_renderTargetView = _graphicsContext->CreateRenderTargetView(desc),
                .m_textureResource = _desc.m_textureResource,
            },
#if !defined(KE_FINAL)
            .m_name = _name.data(),
#endif
        });
    }

    SimplePoolHandle Registry::CreateTextureSrv(
        GraphicsContext* _graphicsContext,
        SimplePoolHandle _texture,
        const KryneEngine::TextureSrvDesc& _desc,
        eastl::string_view _name)
    {
        Resource& resource = m_resources.Get(_texture);

        KryneEngine::TextureSrvDesc desc = _desc;
        desc.m_texture = resource.m_rawTextureData.m_texture;

        return m_resources.AllocateAndInit(Resource {
            .m_type = ResourceType::TextureSrv,
            .m_owned = true,
            .m_textureSrvData = {
                .m_textureSrv = _graphicsContext->CreateTextureSrv(desc),
                .m_textureResource = _texture,
            },
        });
    }

    SimplePoolHandle Registry::GetUnderlyingResource(SimplePoolHandle _resource) const
    {
        const Resource& resource = m_resources.Get(_resource);

        switch (resource.m_type)
        {
        case ResourceType::TextureSrv:
            return resource.m_textureSrvData.m_textureResource;
        case ResourceType::BufferCbv:
            return resource.m_bufferCbvData.m_bufferResource;
        case ResourceType::RenderTargetView:
            return resource.m_renderTargetViewData.m_textureResource;
        case ResourceType::RawTexture:
        case ResourceType::RawBuffer:
        case ResourceType::Sampler:
            return _resource;
        }
    }

    const Resource& Registry::GetResource(SimplePoolHandle _resource) const
    {
        return m_resources.Get( _resource);
    }

    bool Registry::IsRenderTargetView(SimplePoolHandle _resource) const
    {
        return m_resources.Get(_resource).m_type == ResourceType::RenderTargetView;
    }

    RenderTargetViewHandle Registry::GetRenderTargetView(SimplePoolHandle _resource) const
    {
        const Resource& resource = m_resources.Get(_resource);
        VERIFY_OR_RETURN(resource.m_type == ResourceType::RenderTargetView, RenderTargetViewHandle { GenPool::kInvalidHandle });
        return resource.m_renderTargetViewData.m_renderTargetView;
    }

    TextureSrvHandle Registry::GetTextureSrv(SimplePoolHandle _resource) const
    {
        const Resource& resource = m_resources.Get(_resource);
        VERIFY_OR_RETURN(resource.m_type == ResourceType::TextureSrv, TextureSrvHandle { GenPool::kInvalidHandle });
        return resource.m_textureSrvData.m_textureSrv;
    }
} // namespace KryneEngine::Modules::RenderGraph

