/**
 * @file
 * @author Max Godefroy
 * @date 14/11/2024.
 */

#pragma once

#include <KryneEngine/Core/Graphics/Common/Handles.hpp>
#include <KryneEngine/Core/Memory/SimplePool.hpp>

namespace KryneEngine
{
    class GraphicsContext;
    struct TextureCreateDesc;
}

namespace KryneEngine::Modules::RenderGraph
{
    struct RenderTargetViewDesc;
    struct Resource;

    class Registry
    {
        friend class Builder;

    public:
        Registry();
        ~Registry();

    public:
        SimplePoolHandle RegisterRawTexture(TextureHandle _texture, const eastl::string_view& _name = "");
        SimplePoolHandle RegisterRawBuffer(BufferHandle _buffer, const eastl::string_view& _name = "");
        SimplePoolHandle RegisterTextureSrv(
            TextureSrvHandle _textureSrv,
            SimplePoolHandle _textureResource,
            const eastl::string_view& _name = {});
        SimplePoolHandle RegisterCbv(
            BufferCbvHandle _cbv,
            SimplePoolHandle _bufferResource,
            const eastl::string_view& _name = {});
        SimplePoolHandle RegisterRenderTargetView(
            RenderTargetViewHandle _rtv,
            SimplePoolHandle _textureResource,
            const eastl::string_view& _name = {});

        SimplePoolHandle CreateRawTexture(
            GraphicsContext* _graphicsContext,
            const TextureCreateDesc& _desc);
        SimplePoolHandle CreateRenderTargetView(
            GraphicsContext* _graphicsContext,
            const RenderTargetViewDesc& _desc,
            eastl::string_view _name = {});


        [[nodiscard]] SimplePoolHandle GetUnderlyingResource(SimplePoolHandle _resource) const;

        [[nodiscard]] const Resource& GetResource(SimplePoolHandle _resource) const;

        [[nodiscard]] bool IsRenderTargetView(SimplePoolHandle _resource) const;
        [[nodiscard]] RenderTargetViewHandle GetRenderTargetView(SimplePoolHandle _resource) const;

    private:
        SimplePool<Resource, void, true> m_resources;
    };
} // namespace KryneEngine::Modules::RenderGraph

