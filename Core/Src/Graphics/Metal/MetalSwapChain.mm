/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#include "Graphics/Metal/MetalSwapChain.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <QuartzCore/CAMetalLayer.h>

#include "Graphics/Metal/MetalResources.hpp"
#include "KryneEngine/Core/Graphics/Common/ResourceViews/RenderTargetView.hpp"
#include "KryneEngine/Core/Window/Window.hpp"

namespace KryneEngine
{
    MetalSwapChain::MetalSwapChain(
        MTL::Device& _device,
        const GraphicsCommon::ApplicationInfo& _appInfo,
        const Window* _window,
        MetalResources& _resources,
        u8 _initialFrameIndex)
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
            metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        }

        metalLayer.displaySyncEnabled = YES;

        metalLayer.maximumDrawableCount =
            _appInfo.m_displayOptions.m_tripleBuffering == GraphicsCommon::SoftEnable::Disabled
                ? 2
                : 3;

        m_textures.Resize(metalLayer.maximumDrawableCount);
        m_rtvs.Resize(metalLayer.maximumDrawableCount);

        metalLayer.framebufferOnly = YES;

        metalWindow.contentView.layer = metalLayer;
        metalWindow.contentView.wantsLayer = YES;

        m_metalLayer = reinterpret_cast<CA::MetalLayer*>(metalLayer);

        const RenderTargetViewDesc rtvDesc {
            .m_texture = { GenPool::kInvalidHandle },
            .m_format = metalLayer.pixelFormat == MTLPixelFormatBGRA8Unorm_sRGB
                ? TextureFormat::BGRA8_sRGB
                : TextureFormat::BGRA8_UNorm,
        };

        {
            KE_AUTO_RELEASE_POOL;
            m_drawable = m_metalLayer->nextDrawable()->retain();
            KE_ASSERT_FATAL(m_drawable != nullptr);
            for (size_t i = 0; i < metalLayer.maximumDrawableCount; i++)
            {
                m_textures[i] = _resources.RegisterTexture(m_drawable->texture()->retain());
                m_rtvs[i] = _resources.RegisterRtv(rtvDesc, m_drawable->texture()->retain());
            }
        }

        m_index = _initialFrameIndex;
    }

    void MetalSwapChain::Present(CommandList _commandList, u8 _frameIndex)
    {
        _commandList->m_commandBuffer->presentDrawable(m_drawable.get());
    }

    void MetalSwapChain::UpdateNextDrawable(u8 _frameIndex, MetalResources& _resources)
    {
        KE_AUTO_RELEASE_POOL;
        CA::MetalDrawable* drawable = m_metalLayer->nextDrawable()->retain();
        m_drawable.reset(drawable);
        KE_ASSERT_FATAL(m_drawable != nullptr);
        _resources.UpdateSystemTexture(m_textures[_frameIndex], drawable->texture()->retain());
        _resources.UpdateSystemTexture(m_rtvs[_frameIndex], drawable->texture()->retain());

        m_index = (_frameIndex + 1) % m_textures.Size();
    }
} // namespace KryneEngine