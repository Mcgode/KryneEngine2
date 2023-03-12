/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#include "Dx12GraphicsContext.hpp"
#include "HelperFunctions.hpp"
#include <Graphics/Common/Window.hpp>
#include "Dx12SwapChain.hpp"

namespace KryneEngine
{
    Dx12GraphicsContext::Dx12GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo)
        : m_appInfo(_appInfo)
    {
        Assert(m_appInfo.IsDirectX12Api());

        ComPtr<IDXGIFactory4> factory4;
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

        Dx12Assert(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory4)));

        _CreateDevice(factory4.Get());
        _CreateCommandQueues();

        if (m_appInfo.m_features.m_present)
        {
            m_window = eastl::make_unique<Window>(m_appInfo);
            m_swapChain = eastl::make_unique<Dx12SwapChain>(m_appInfo,
                                                            m_window.get(),
                                                            factory4.Get(),
                                                            m_device.Get(),
                                                            m_directQueue.Get());
        }
    }

    Dx12GraphicsContext::~Dx12GraphicsContext() = default;

    Window *Dx12GraphicsContext::GetWindow() const
    {
        return m_window.get();
    }

    void Dx12GraphicsContext::_CreateDevice(IDXGIFactory4 *_factory4)
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        _FindAdapter(_factory4, &hardwareAdapter);

        Dx12Assert(D3D12CreateDevice(hardwareAdapter.Get(),
                                     Dx12Converters::GetFeatureLevel(m_appInfo),
                                     IID_PPV_ARGS(&m_device)));
    }

    void Dx12GraphicsContext::_FindAdapter(IDXGIFactory4 *_factory, IDXGIAdapter1 **_adapter)
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

    void Dx12GraphicsContext::_CreateCommandQueues()
    {
        const auto& features = m_appInfo.m_features;

        if (features.m_graphics)
        {
            D3D12_COMMAND_QUEUE_DESC directQueueDesc {};
            directQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            directQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            directQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            directQueueDesc.NodeMask = 0;

            Dx12Assert(m_device->CreateCommandQueue(&directQueueDesc, IID_PPV_ARGS(&m_directQueue)));
        }

        if ((!features.m_graphics || features.m_asyncCompute) && features.m_compute )
        {
            D3D12_COMMAND_QUEUE_DESC computeQueueDesc {};
            computeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            computeQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            computeQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            computeQueueDesc.NodeMask = 0;

            Dx12Assert(m_device->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&m_computeQueue)));
        }

        if ((!features.m_graphics && !features.m_compute || features.m_transferQueue) && features.m_transfer)
        {
            D3D12_COMMAND_QUEUE_DESC copyQueueDesc {};
            copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            copyQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            copyQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            copyQueueDesc.NodeMask = 0;

            Dx12Assert(m_device->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&m_copyQueue)));
        }
    }
} // KryneEngine