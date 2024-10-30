/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#include "MetalSwapChain.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <QuartzCore/CAMetalLayer.h>

#include <Window/Window.hpp>

namespace KryneEngine
{
    MetalSwapChain::MetalSwapChain(
        MTL::Device& _device,
        const GraphicsCommon::ApplicationInfo& _appInfo,
        const Window* _window)
    {
        auto* metalWindow = static_cast<NSWindow*>(glfwGetCocoaWindow(_window->GetGlfwWindow()));

        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        metalLayer.device = (__bridge id<MTLDevice>)&_device;
        if (_appInfo.m_displayOptions.m_sRgbPresent == GraphicsCommon::SoftEnable::Disabled)
        {
            metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        }
        else
        {
            metalLayer.pixelFormat = MTLPixelFormatRGBA8Unorm_sRGB;
        }

        metalLayer.maximumDrawableCount =
            _appInfo.m_displayOptions.m_tripleBuffering == GraphicsCommon::SoftEnable::Disabled
                ? 2
                : 3;

        m_drawables.Resize(metalLayer.maximumDrawableCount);

        metalLayer.framebufferOnly = YES;

        metalWindow.contentView.layer = metalLayer;
        metalWindow.contentView.wantsLayer = YES;

        m_metalLayer = reinterpret_cast<CA::MetalLayer*>(metalLayer);

        for (size_t i = 0; i < m_drawables.Size(); i++)
        {
            m_drawables.Init(i, m_metalLayer->nextDrawable());
            KE_ASSERT_FATAL(m_drawables[i] != nullptr);
        }
    }

    void MetalSwapChain::Present(CommandList _commandList, u8 _frameIndex)
    {
        _commandList->presentDrawable(m_drawables[_frameIndex].get());
    }

    void MetalSwapChain::UpdateNextDrawable(u8 _frameIndex)
    {
        m_drawables[_frameIndex].reset(m_metalLayer->nextDrawable());
        KE_ASSERT_FATAL(m_drawables[_frameIndex] != nullptr);
    }
} // namespace KryneEngine