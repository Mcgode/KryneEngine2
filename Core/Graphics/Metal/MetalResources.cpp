/**
 * @file
 * @author Max Godefroy
 * @date 30/10/2024.
 */

#include "MetalResources.hpp"

#include <Graphics/Common/ResourceViews/RenderTargetView.hpp>
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
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
        rtvHot->m_texture = _texture;
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
            rtvHotData->m_texture = _newTexture;
        }
    }
} // namespace KryneEngine