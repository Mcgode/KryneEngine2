/**
 * @file
 * @author Max Godefroy
 * @date 14/11/2024.
 */

#pragma once

#include <Graphics/Common/Handles.hpp>
#include <Memory/SimplePool.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    struct Resource;

    class Registry
    {
        friend class Builder;

    public:
        Registry();
        ~Registry();

    public:
        SimplePoolHandle RegisterRawTexture(TextureHandle _texture, const eastl::string_view& _name = {});
        SimplePoolHandle RegisterRawBuffer(BufferHandle _buffer, const eastl::string_view& _name = {});
        SimplePoolHandle RegisterTextureSrv(
            TextureSrvHandle _textureSrv,
            SimplePoolHandle _textureResource,
            const eastl::string_view& _name = {});

        SimplePoolHandle GetUnderlyingResource(SimplePoolHandle _resource) const;

    private:
        SimplePool<Resource, void, true> m_resources;
    };
} // namespace KryneEngine::Modules::RenderGraph

