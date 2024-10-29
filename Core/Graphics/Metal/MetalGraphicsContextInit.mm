/**
 * @file
 * @author Max Godefroy
 * @date 28/10/2024.
 */

#include "MetalGraphicsContext.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <QuartzCore/CAMetalLayer.h>

#include <Window/Window.hpp>

namespace KryneEngine
{
    MetalGraphicsContext::MetalGraphicsContext(
            const GraphicsCommon::ApplicationInfo& _appInfo,
            const Window* _window,
            u64 _initialFrameId)
                : m_applicationInfo(_appInfo)
    {
        m_device = MTL::CreateSystemDefaultDevice();

        KE_ASSERT_FATAL(!(_appInfo.m_features.m_present ^ (_window != nullptr)));

        if (_appInfo.m_features.m_present)
        {
            auto* metalWindow = static_cast<NSWindow*>(glfwGetCocoaWindow(_window->GetGlfwWindow()));

            CAMetalLayer* metalLayer = [CAMetalLayer layer];
            metalLayer.device = (__bridge id<MTLDevice>)m_device;
            if (_appInfo.m_displayOptions.m_sRgbPresent == GraphicsCommon::SoftEnable::Disabled)
            {
                metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
            }
            else
            {
                metalLayer.pixelFormat = MTLPixelFormatRGBA8Unorm_sRGB;
            }

            metalWindow.contentView.layer = metalLayer;
            metalWindow.contentView.wantsLayer = YES;
        }
    }

    MetalGraphicsContext::~MetalGraphicsContext()
    {
        m_device->release();
        m_device = nullptr;
    }
}