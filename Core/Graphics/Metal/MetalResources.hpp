/**
 * @file
 * @author Max Godefroy
 * @date 30/10/2024.
 */

#pragma once

#include <EASTL/fixed_vector.h>
#include <Graphics/Common/Enums.hpp>
#include <Graphics/Common/Handles.hpp>
#include <Graphics/Common/RenderPass.hpp>
#include <Graphics/Metal/MetalHeaders.hpp>
#include <Memory/GenerationalPool.hpp>

namespace KryneEngine
{
    struct BufferCreateDesc;
    struct RenderTargetViewDesc;
    struct RenderPassDesc;

    class MetalResources
    {
        friend class MetalGraphicsContext;

    public:
        MetalResources();
        ~MetalResources();

    public:
        BufferHandle CreateBuffer(MTL::Device& _device, const BufferCreateDesc& _desc);
        bool DestroyBuffer(BufferHandle _buffer);

    private:
        struct BufferHotData
        {
            NsPtr<MTL::Buffer> m_buffer;
        };

        struct BufferColdData
        {
            MTL::ResourceOptions m_options;
        };

        GenerationalPool<BufferHotData, BufferColdData> m_buffers;

    public:
        TextureHandle RegisterTexture(MTL::Texture* _texture);
        bool UnregisterTexture(TextureHandle _handle);
        void UpdateSystemTexture(TextureHandle _handle, MTL::Texture* _texture);

    private:
        struct TextureHotData
        {
            NsPtr<MTL::Texture> m_texture;
            bool m_isSystemTexture;
        };

        GenerationalPool<TextureHotData> m_textures;

    public:
        RenderTargetViewHandle RegisterRtv(const RenderTargetViewDesc& _desc);
        bool UnregisterRtv(RenderTargetViewHandle _handle);

        RenderTargetViewHandle RegisterRtv(const RenderTargetViewDesc& _desc, MTL::Texture* _texture);
        void UpdateSystemTexture(RenderTargetViewHandle _handle, MTL::Texture* _newTexture);

    private:
        struct RtvHotData
        {
            NsPtr<MTL::Texture> m_texture;
            bool m_isSystemTexture;
        };

        struct RtvColdData
        {
            TextureFormat m_pixelFormat;
            u16 m_slice;
            u16 m_depthSlice;
            u8 m_mipLevel;
            TexturePlane m_plane;
        };

        GenerationalPool<RtvHotData, RtvColdData> m_renderTargetViews;

    public:
        RenderPassHandle CreateRenderPassDescriptor(const RenderPassDesc& _desc);
        bool DestroyRenderPassDescriptor(RenderPassHandle _handle);

    private:
        struct RenderPassHotData
        {
            NsPtr<MTL::RenderPassDescriptor> m_descriptor;

            struct SystemRtv
            {
                RenderTargetViewHandle m_handle;
                u8 m_index;
            };
            eastl::fixed_vector<SystemRtv, 1> m_systemRtvs;
        };

        struct RenderPassColdData
        {
            eastl::fixed_vector<TextureFormat, RenderPassDesc::kMaxSupportedColorAttachments, false> m_colorFormats;
            TextureFormat m_depthStencilFormat;
        };

        GenerationalPool<RenderPassHotData, RenderPassColdData> m_renderPasses;
    };
} // namespace KryneEngine
