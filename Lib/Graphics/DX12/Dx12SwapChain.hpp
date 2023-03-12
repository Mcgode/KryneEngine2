/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#pragma once

#include <Common/Arrays.hpp>
#include "Dx12Headers.hpp"

namespace KryneEngine
{
    class Window;

    class Dx12SwapChain
    {
        friend class Dx12GraphicsContext;

    public:
        Dx12SwapChain(const GraphicsCommon::ApplicationInfo &_appInfo,
                      Window *_processWindow,
                      IDXGIFactory4 *_factory,
                      ID3D12Device *_device,
                      ID3D12CommandQueue *_directQueue);

    private:
        ComPtr<IDXGISwapChain3> m_swapChain;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

        DynamicArray<ComPtr<ID3D12Resource>> m_renderTargets;

        u32 m_rtvDescriptorSize = 0;
        u8 m_currentFrame;
    };
} // KryneEngine