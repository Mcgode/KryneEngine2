/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#pragma once

#include "KryneEngine/Core/Graphics/Handles.hpp"

namespace KryneEngine::Modules::RenderGraph
{
    enum class ResourceType
    {
        RawTexture,
        Buffer,
        Sampler,
        TextureSrv,
        BufferView,
        RenderTargetView,
    };

    struct RawTextureData
    {
        TextureHandle m_texture;
    };

    struct BufferData
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

    struct BufferViewData
    {
        BufferViewHandle m_bufferView;
        SimplePoolHandle m_bufferResource;
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
            BufferData m_bufferData;
            SamplerData m_samplerData;
            TextureSrvData m_textureSrvData;
            BufferViewData m_bufferViewData;
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
