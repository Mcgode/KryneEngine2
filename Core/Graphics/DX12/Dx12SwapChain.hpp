/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#pragma once

#include <Common/Arrays.hpp>
#include "Dx12Headers.hpp"
#include "Dx12Resources.h"

namespace KryneEngine
{
    class Window;

    class Dx12SwapChain
    {
        friend class Dx12GraphicsContext;

    public:
        Dx12SwapChain(const GraphicsCommon::ApplicationInfo &_appInfo, Window *_processWindow,
                      IDXGIFactory4 *_factory, ID3D12Device *_device, ID3D12CommandQueue *_directQueue,
                      KryneEngine::Dx12Resources &_resources);

        ~Dx12SwapChain();

        [[nodiscard]] u8 GetBackBufferIndex() const
        {
	        return m_swapChain->GetCurrentBackBufferIndex();
        }

        void Present() const;

        void Destroy(Dx12Resources& _resources);

    private:
        ComPtr<IDXGISwapChain3> m_swapChain;

        DynamicArray<GenPool::Handle> m_renderTargetTextures;
        DynamicArray<GenPool::Handle> m_renderTargetViews;

        u8 m_currentFrame;
    };
} // KryneEngine