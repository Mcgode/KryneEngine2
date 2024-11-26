/**
 * @file
 * @author Max Godefroy
 * @date 14/11/2024.
 */

#include "RenderGraph/Registry.hpp"

#include <KryneEngine/Core/Memory/SimplePool.inl>
#include <RenderGraph/Resource.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    Registry::Registry() = default;
    Registry::~Registry() = default;

    SimplePoolHandle Registry::RegisterRawTexture(TextureHandle _texture, const eastl::string_view& _name)
    {
        const SimplePoolHandle handle = m_resources.AllocateAndInit(Resource {
            .m_type = ResourceType::RawTexture,
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

    SimplePoolHandle Registry::GetUnderlyingResource(SimplePoolHandle _resource) const
    {
        const Resource& resource = m_resources.Get(_resource);

        switch (resource.m_type)
        {
        case ResourceType::TextureSrv:
            return resource.m_textureSrvData.m_textureResource;
        case ResourceType::RawTexture:
        case ResourceType::RawBuffer:
        case ResourceType::Sampler:
            return _resource;
        }
    }
} // namespace KryneEngine::Modules::RenderGraph

