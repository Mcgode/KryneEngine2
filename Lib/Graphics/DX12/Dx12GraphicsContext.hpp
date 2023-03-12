/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#pragma once

#include "Dx12Headers.hpp"
#include <Graphics/Common/GraphicsCommon.hpp>
#include <EASTL/unique_ptr.h>

namespace KryneEngine
{
    class Window;
    class Dx12SwapChain;

    class Dx12GraphicsContext
    {
    public:
        explicit Dx12GraphicsContext(const GraphicsCommon::ApplicationInfo& _appInfo);

        ~Dx12GraphicsContext();

        [[nodiscard]] Window* GetWindow() const;

    private:
        GraphicsCommon::ApplicationInfo m_appInfo;

        eastl::unique_ptr<Window> m_window;

        ComPtr<ID3D12Device> m_device;

        ComPtr<ID3D12CommandQueue> m_directQueue;
        ComPtr<ID3D12CommandQueue> m_computeQueue;
        ComPtr<ID3D12CommandQueue> m_copyQueue;

        eastl::unique_ptr<Dx12SwapChain> m_swapChain;

        void _CreateDevice(IDXGIFactory4 *_factory4);
        void _FindAdapter(IDXGIFactory4* _factory, IDXGIAdapter1** _adapter);

        void _CreateCommandQueues();
    };
} // KryneEngine