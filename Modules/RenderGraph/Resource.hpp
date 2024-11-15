/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#pragma once

#include <Graphics/Common/Handles.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    enum class ResourceType
    {
        RawTexture,
        RawBuffer,
        Sampler,
        TextureSrv,
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
    };

    struct Resource
    {
        ResourceType m_type;
        union {
            RawTextureData m_rawTextureData;
            RawBufferData m_rawBufferData;
            SamplerData m_samplerData;
            TextureSrvData m_textureSrvData;
        };
#if !defined(KE_FINAL)
        eastl::string m_name;
#endif
    };
} // namespace KryneEngine::Modules::RenderGraph
