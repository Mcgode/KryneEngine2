/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#pragma once

#import <Common/Arrays.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Graphics/Metal/MetalHeaders.hpp>
#include <Graphics/Metal/MetalTypes.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace KryneEngine
{
    class Window;

    class MetalSwapChain
    {
        friend class MetalGraphicsContext;

    public:
        MetalSwapChain(MTL::Device& _device, const GraphicsCommon::ApplicationInfo& _appInfo, const Window* _window);

        void Present(CommandList _commandList, u8 _frameIndex);

        void UpdateNextDrawable(u8 _frameIndex);

    private:
        CA::MetalLayer* m_metalLayer;
        DynamicArray<NsPtr<CA::MetalDrawable>> m_drawables;
    };
} // namespace KryneEngine
