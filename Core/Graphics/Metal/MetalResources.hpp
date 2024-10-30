/**
 * @file
 * @author Max Godefroy
 * @date 30/10/2024.
 */

#pragma once

#include <Graphics/Common/Enums.hpp>
#include <Graphics/Common/Handles.hpp>
#include <Graphics/Metal/MetalHeaders.hpp>
#include <Memory/GenerationalPool.hpp>

namespace KryneEngine
{
    struct RenderTargetViewDesc;
    struct RenderPassDesc;

    class MetalResources
    {
    public:
        RenderTargetViewHandle RegisterRtv(const RenderTargetViewDesc& _desc);
        bool UnregisterRtv(RenderTargetViewHandle _handle);

        RenderTargetViewHandle RegisterRtv(const RenderTargetViewDesc& _desc, MTL::Texture* _texture);
        void UpdateSystemTexture(RenderTargetViewHandle _handle, MTL::Texture* _newTexture);

    private:
        struct RtvHotData
        {
            NsPtr<MTL::Texture> m_texture;
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
    };
} // namespace KryneEngine
