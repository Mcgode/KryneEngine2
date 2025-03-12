/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#pragma once

#include <KryneEngine/Core/Graphics/Common/Handles.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    enum class ResourceType
    {
        RawTexture,
        RawBuffer,
        Sampler,
        TextureSrv,
        RenderTargetView,
    };

    struct RawTextureData
    {
        TextureHandle m_texture;
    };

    struct RawBufferData
    {
        BufferHandle m_buffer;
    };

    struct SamplerData
    {
        SamplerHandle m_sampler;
    };

    struct TextureSrvData
    {
        TextureSrvHandle m_textureSrv;
        SimplePoolHandle m_textureResource;
    };

    struct RenderTargetViewData
    {
        RenderTargetViewHandle m_renderTargetView;
        SimplePoolHandle m_textureResource;
    };

    struct Resource
    {
        ResourceType m_type;
        bool m_owned;
        union {
            RawTextureData m_rawTextureData;
            RawBufferData m_rawBufferData;
            SamplerData m_samplerData;
            TextureSrvData m_textureSrvData;
            RenderTargetViewData m_renderTargetViewData;
        };
#if !defined(KE_FINAL)
        eastl::string m_name;
#endif

        [[nodiscard]] bool IsTexture() const
        {
            switch (m_type)
            {
                case ResourceType::RawTexture:
                case ResourceType::TextureSrv:
                case ResourceType::RenderTargetView:
                    return true;
                default:
                    return false;
            }
        }
    };
} // namespace KryneEngine::Modules::RenderGraph
