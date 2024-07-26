/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#include "Dx12SwapChain.hpp"
#include "Dx12GraphicsContext.hpp"
#include "HelperFunctions.hpp"
#include <Graphics/Common/Window.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Graphics/Common/RenderTargetView.hpp>

namespace KryneEngine
{
    Dx12SwapChain::Dx12SwapChain(const GraphicsCommon::ApplicationInfo &_appInfo,
                                 Window *_processWindow,
                                 IDXGIFactory4 *_factory,
                                 ID3D12Device *_device,
                                 ID3D12CommandQueue *_directQueue,
                                 Dx12Resources &_resources)
    {
        const auto& displayInfo = _appInfo.m_displayOptions;

        u32 imageCount = 2;
        if (displayInfo.m_tripleBuffering != GraphicsCommon::SoftEnable::Disabled)
        {
            imageCount++;
        }

        // sRGB format is set by the RTV
        const auto format = DXGI_FORMAT_B8G8R8A8_UNORM;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc {};
        swapChainDesc.BufferCount = imageCount;
        swapChainDesc.Width = displayInfo.m_width;
        swapChainDesc.Height = displayInfo.m_height;
        swapChainDesc.Format = format;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1; // Disable MultiSampling

        ComPtr<IDXGISwapChain1> swapChain;
        auto hwndWindow = glfwGetWin32Window(_processWindow->GetGlfwWindow());
        Dx12Assert(_factory->CreateSwapChainForHwnd(
            _directQueue,
            hwndWindow,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        ));

        Dx12Assert(_factory->MakeWindowAssociation(hwndWindow, DXGI_MWA_NO_ALT_ENTER));

        Dx12Assert(swapChain.As(&m_swapChain));
#if !defined(KE_FINAL)
        Dx12SetName(m_swapChain.Get(), L"Swap Chain");
#endif

        m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();

        // Create frame render targets
        {
            m_renderTargetTextures.Resize(imageCount);
            m_renderTargetViews.Resize(imageCount);

            ID3D12Resource* renderTargetTexture = nullptr;
            for (u32 i = 0; i < imageCount; i++)
            {
                Dx12Assert(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargetTexture)));
#if !defined(KE_FINAL)
                Dx12SetName(renderTargetTexture, L"SwapChain Render Target Texture %d", i);
#endif
                const auto textureHandle = _resources.RegisterTexture(renderTargetTexture, nullptr);
                m_renderTargetTextures.Init(i, textureHandle);

                const RenderTargetViewDesc rtvDesc {
                    .m_textureHandle = textureHandle,
                    .m_format = displayInfo.m_sRgbPresent == GraphicsCommon::SoftEnable::Disabled
                            ? TextureFormat::BGRA8_UNorm
                            : TextureFormat::BGRA8_sRGB
                };
                m_renderTargetViews.Init(i, _resources.CreateRenderTargetView(rtvDesc, _device));
            }
        }
    }

    Dx12SwapChain::~Dx12SwapChain()
    {
        KE_ASSERT(m_swapChain == nullptr);
    }

    void Dx12SwapChain::Present() const
    {
        Dx12Assert(m_swapChain->Present(0, 0));
    }

    void Dx12SwapChain::Destroy(Dx12Resources &_resources)
    {
        for (const auto handle: m_renderTargetViews)
        {
            KE_ASSERT_MSG(_resources.FreeRenderTargetView(handle), "Handle was invalid. It shouldn't. Something went wrong with the lifecycle.");
        }
        m_renderTargetViews.Clear();

        for (const auto handle: m_renderTargetTextures)
        {
            // Free the texture from the gen pool, but don't do a release of the ID3D12Resource, as it's handled
            // by the swapchain
            KE_ASSERT_MSG(_resources.ReleaseTexture(handle, false), "Handle was invalid. It shouldn't. Something went wrong with the lifecycle.");
        }
        m_renderTargetTextures.Clear();

        SafeRelease(m_swapChain);
    }
} // KryneEngine