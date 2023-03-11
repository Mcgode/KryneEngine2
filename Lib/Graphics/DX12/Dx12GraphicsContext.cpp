/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#include "Dx12GraphicsContext.hpp"
#include "HelperFunctions.hpp"
#include <Graphics/Common/Window.hpp>

namespace KryneEngine
{
    Dx12GraphicsContext::Dx12GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo)
        : m_appInfo(_appInfo)
    {
        Assert(m_appInfo.IsDirectX12Api());

        if (m_appInfo.m_features.m_present)
        {
            m_window = eastl::make_unique<Window>(Window::Params());
        }

        _CreateDevice();
    }

    Window *Dx12GraphicsContext::GetWindow() const
    {
        return m_window.get();
    }

    void Dx12GraphicsContext::_CreateDevice()
    {
        UINT dxgiFactoryFlags = 0;

#if !defined(KE_FINAL)
        if (m_appInfo.m_features.m_validationLayers)
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif

        ComPtr<IDXGIFactory4> factory4;
        Dx12Assert(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory4)));

        ComPtr<IDXGIAdapter1> hardwareAdapter;
        _FindAdapter(factory4.Get(), &hardwareAdapter);

        Dx12Assert(D3D12CreateDevice(hardwareAdapter.Get(),
                                     Dx12Converters::GetFeatureLevel(m_appInfo),
                                     IID_PPV_ARGS(&m_device)));
    }

    void Dx12GraphicsContext::_FindAdapter(IDXGIFactory1 *_factory, IDXGIAdapter1 **_adapter)
    {
        *_adapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;
        ComPtr<IDXGIFactory6> factory6;

        if (Dx12Verify(_factory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (u32 adapterIndex = 0;
                 SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex,
                                                                DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                                IID_PPV_ARGS(&adapter)));
                 adapterIndex++)
            {
                DXGI_ADAPTER_DESC1 adapterDesc;
                adapter->GetDesc1(&adapterDesc);

                if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't use software adapter
                    continue;
                }

                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(),
                                                Dx12Converters::GetFeatureLevel(m_appInfo),
                                                _uuidof(ID3D12Device),
                                                nullptr)))
                {
                    break;
                }
            }
        }

        *_adapter = adapter.Detach();
    }
} // KryneEngine