/**
 * @file
 * @author Max Godefroy
 * @date 04/08/2024.
 */

#pragma once

#include <Memory/GenerationalPool.hpp>

namespace KryneEngine
{
#define KE_GRAPHICS_DECLARE_HANDLE(HandleName) struct HandleName                                \
    {                                                                                           \
        GenPool::Handle m_handle = GenPool::kInvalidHandle;                                     \
                                                                                                \
        HandleName& operator=(GenPool::Handle _other) { m_handle = _other; return *this; }      \
        bool operator==(GenPool::Handle _other) const { return m_handle == _other; }            \
    }

    KE_GRAPHICS_DECLARE_HANDLE(BufferHandle);
    KE_GRAPHICS_DECLARE_HANDLE(TextureHandle);
    KE_GRAPHICS_DECLARE_HANDLE(TextureSrvHandle);
    KE_GRAPHICS_DECLARE_HANDLE(RenderTargetViewHandle);
    KE_GRAPHICS_DECLARE_HANDLE(RenderPassHandle);
    KE_GRAPHICS_DECLARE_HANDLE(ShaderModuleHandle);
    KE_GRAPHICS_DECLARE_HANDLE(GraphicsPipelineHandle);
}
