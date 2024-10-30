/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#pragma once

#import <Common/Arrays.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Graphics/Common/Handles.hpp>
#include <Graphics/Metal/MetalHeaders.hpp>
#include <Graphics/Metal/MetalTypes.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace KryneEngine
{
    class MetalResources;
    class Window;

    class MetalSwapChain
    {
        friend class MetalGraphicsContext;

    public:
        MetalSwapChain(
            MTL::Device& _device,
            const GraphicsCommon::ApplicationInfo& _appInfo,
            const Window* _window,
            MetalResources& _resources,
            u8 _initialFrameIndex);

        void Present(CommandList _commandList, u8 _frameIndex);

        void UpdateNextDrawable(u8 _frameIndex, MetalResources& _resources);

    private:
        CA::MetalLayer* m_metalLayer;
        DynamicArray<NsPtr<CA::MetalDrawable>> m_drawables;
        DynamicArray<TextureHandle> m_textures;
        DynamicArray<RenderTargetViewHandle> m_rtvs;
        u8 m_index;
    };
} // namespace KryneEngine
