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

namespace KryneEngine
{
    Dx12SwapChain::Dx12SwapChain(const GraphicsCommon::ApplicationInfo &_appInfo,
                                 Window *_processWindow,
                                 IDXGIFactory4 *_factory,
                                 ID3D12Device *_device,
                                 ID3D12CommandQueue *_directQueue)
    {
        const auto& displayInfo = _appInfo.m_displayOptions;

        {
            u32 imageCount = 2;
            if (displayInfo.m_tripleBuffering != GraphicsCommon::SoftEnable::Disabled)
            {
                imageCount++;
            }
            m_renderTargets = DynamicArray<ComPtr<ID3D12Resource>>(imageCount);
            m_renderTargets.InitAll();
        }

        const auto format = displayInfo.m_sRgbPresent == GraphicsCommon::SoftEnable::Disabled
                ? DXGI_FORMAT_B8G8R8A8_UNORM
                : DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc {};
        swapChainDesc.BufferCount = m_renderTargets.Size();
        swapChainDesc.Width = displayInfo.m_width;
        swapChainDesc.Height = displayInfo.m_height;

        // sRGB will come from render target. See:
        // - https://gamedev.net/forums/topic/670546-d3d12srgb-buffer-format-for-swap-chain/5243987/
        // - https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb173064(v=vs.85)
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1; // Disable MultiSampling

        ComPtr<IDXGISwapChain1> swapChain;
        auto hwndWindow = glfwGetWin32Window(_processWindow->GetGlfwWindow());
        Dx12Assert(_factory->CreateSwapChainForHwnd(_directQueue,
                                                    hwndWindow,
                                                    &swapChainDesc,
                                                    nullptr,
                                                    nullptr,
                                                    &swapChain));

        Dx12Assert(_factory->MakeWindowAssociation(hwndWindow, DXGI_MWA_NO_ALT_ENTER));

        Dx12Assert(swapChain.As(&m_swapChain));

        m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();

        // Create heap
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
            rtvHeapDesc.NumDescriptors = m_renderTargets.Size();
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            rtvHeapDesc.NodeMask = 0;

            Dx12Assert(_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

            m_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        // Create frame render targets
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = format;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            for (u32 i = 0; i < m_renderTargets.Size(); i++)
            {
                Dx12Assert(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
                _device->CreateRenderTargetView(m_renderTargets[i].Get(), &rtvDesc, rtvHandle);
                rtvHandle.Offset(1, m_rtvDescriptorSize);
            }
        }
    }
} // KryneEngine