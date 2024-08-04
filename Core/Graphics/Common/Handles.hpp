/**
 * @file
 * @author Max Godefroy
 * @date 04/08/2024.
 */

#pragma once

#include <Memory/GenerationalPool.hpp>

namespace KryneEngine
{
    struct BufferHandle
    {
        GenPool::Handle m_handle = GenPool::kInvalidHandle;

        BufferHandle& operator=(GenPool::Handle _other) { m_handle = _other; return *this; }
        bool operator==(GenPool::Handle _other) const { return m_handle == _other; }
    };

    struct TextureHandle
    {
        GenPool::Handle m_handle = GenPool::kInvalidHandle;

        TextureHandle& operator=(GenPool::Handle _other) { m_handle = _other; return *this; }
        bool operator==(GenPool::Handle _other) const { return m_handle == _other; }
    };

    struct TextureSrvHandle
    {
        GenPool::Handle m_handle = GenPool::kInvalidHandle;

        TextureSrvHandle& operator=(GenPool::Handle _other) { m_handle = _other; return *this; }
        bool operator==(GenPool::Handle _other) const { return m_handle == _other; }
    };

    struct RenderTargetViewHandle
    {
        GenPool::Handle m_handle = GenPool::kInvalidHandle;

        RenderTargetViewHandle& operator=(GenPool::Handle _other) { m_handle = _other; return *this; }
        bool operator==(GenPool::Handle _other) const { return m_handle == _other; }
    };

    struct RenderPassHandle
    {
        GenPool::Handle m_handle = GenPool::kInvalidHandle;

        RenderPassHandle& operator=(GenPool::Handle _other) { m_handle = _other; return *this; }
        bool operator==(GenPool::Handle _other) const { return m_handle == _other; }
    };
}
