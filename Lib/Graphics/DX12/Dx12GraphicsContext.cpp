/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#include "Dx12GraphicsContext.hpp"
#include <dxgidebug.h>
#include "HelperFunctions.hpp"
#include <Graphics/Common/Window.hpp>
#include "Dx12SwapChain.hpp"

namespace KryneEngine
{
    Dx12GraphicsContext::Dx12GraphicsContext(const GraphicsCommon::ApplicationInfo& _appInfo)
        : m_appInfo(_appInfo)
    {
        KE_ASSERT(m_appInfo.IsDirectX12Api());

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

            m_frameContextCount = m_swapChain->m_renderTargets.Size();

            m_frameContextIndex = m_swapChain->GetBackBufferIndex();
        }
        else
        {
            // If no display, remain on double buffering.
            m_frameContextCount = 2;

            m_frameContextIndex = 0;
        }

        m_frameContexts.Resize(m_frameContextCount);
        m_frameContexts.InitAll(m_device.Get(),
                                m_directQueue != nullptr,
                                m_computeQueue != nullptr,
                                m_copyQueue != nullptr);

        // Create the frame fence
        Dx12Assert(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_frameFence)));
#if !defined(KE_FINAL)
        Dx12SetName(m_frameFence.Get(), L"Frame fence");
#endif
        m_frameFenceEvent = CreateEvent(nullptr, false, false, nullptr);
        KE_ASSERT(m_frameFenceEvent != nullptr);

        if (m_appInfo.m_features.m_renderPipelineShaders)
        {
            RpsD3D12RuntimeDeviceCreateInfo createInfo;
            createInfo.pDeviceCreateInfo = nullptr;
            createInfo.pRuntimeCreateInfo = nullptr;
            createInfo.pD3D12Device = m_device.Get();
        }
    }

    Dx12GraphicsContext::~Dx12GraphicsContext()
    {
        rpsDeviceDestroy(m_rpsDevice);

#if !defined(KE_FINAL)
        if (m_validationLayerMessageCallbackHandle != 0)
        {
            ComPtr<ID3D12InfoQueue1> infoQueue;
            if (SUCCEEDED(m_device->QueryInterface<ID3D12InfoQueue1>(&infoQueue)))
            {
                infoQueue->UnregisterMessageCallback(m_validationLayerMessageCallbackHandle);
            }
        }
#endif

        CloseHandle(m_frameFenceEvent);
        SafeRelease(m_frameFence);

        m_frameContexts.Clear();

        m_swapChain.release();

        SafeRelease(m_copyQueue);
        SafeRelease(m_computeQueue);
        SafeRelease(m_directQueue);

        SafeRelease(m_device);

        if (m_appInfo.m_features.m_validationLayers)
        {
            IDXGIDebug* debugDev;
            Dx12Assert(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugDev)));
            Dx12Assert(debugDev->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL));
        }
    }

    Window *Dx12GraphicsContext::GetWindow() const
    {
        return m_window.get();
    }

    void Dx12GraphicsContext::EndFrame(u64 _frameId)
    {
        auto& frameContext = _GetFrameContext();

        // Execute the command lists
        ID3D12CommandQueue* queue = nullptr;
        {
            const auto executeCommands = [&](ID3D12CommandQueue* _queue, Dx12FrameContext::CommandAllocationSet& _allocationSet)
            {
                if (_queue != nullptr && !_allocationSet.m_usedCommandLists.empty())
                {
                    queue = _queue;
                    auto** commandLists = reinterpret_cast<ID3D12CommandList**>(_allocationSet.m_usedCommandLists.data());
                    _queue->ExecuteCommandLists(_allocationSet.m_usedCommandLists.size(), commandLists);
                }
            };

            executeCommands(m_copyQueue.Get(), frameContext.m_copyCommandAllocationSet);
            executeCommands(m_computeQueue.Get(), frameContext.m_computeCommandAllocationSet);
            executeCommands(m_directQueue.Get(), frameContext.m_directCommandAllocationSet);
        }

        // Present the frame (if applicable)
        if (m_swapChain != nullptr)
        {
            m_swapChain->Present();
        }

        // Increment fence signal
        if (queue != nullptr)
        {
            Dx12Assert(queue->Signal(m_frameFence.Get(), _frameId));
        }
        else
        {
            // If there was no submitted command list, simply wait for the previous frame and set the frame as completed.
            WaitForFrame(_frameId - 1);
            Dx12Assert(m_frameFence->Signal(_frameId));
        }
        _GetFrameContext().m_frameId = _frameId;

        // Retrieve next frame index
        if (m_swapChain != nullptr)
        {
            m_frameContextIndex = m_swapChain->GetBackBufferIndex();
        }
        else
        {
            m_frameContextIndex = (m_frameContextIndex + 1) % m_frameContextCount;
        }

        // Wait for the previous frame with this index.
        WaitForFrame(_GetFrameContext().m_frameId);
    }

    void Dx12GraphicsContext::WaitForFrame(u64 _frameId) const
    {
        if (m_frameFence->GetCompletedValue() < _frameId)
        {
            Dx12Assert(m_frameFence->SetEventOnCompletion(_frameId, m_frameFenceEvent));
            WaitForSingleObject(m_frameFenceEvent, INFINITE);
        }
    }

    void Dx12GraphicsContext::_CreateDevice(IDXGIFactory4 *_factory4)
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        _FindAdapter(_factory4, &hardwareAdapter);

        Dx12Assert(D3D12CreateDevice(hardwareAdapter.Get(),
                                     Dx12Converters::GetFeatureLevel(m_appInfo),
                                     IID_PPV_ARGS(&m_device)));
#if !defined(KE_FINAL)
        Dx12SetName(m_device.Get(), L"Device");
#endif

#if !defined(KE_FINAL)
        if (m_appInfo.m_features.m_validationLayers)
        {
            ComPtr<ID3D12InfoQueue1> infoQueue;
            if (SUCCEEDED(m_device->QueryInterface<ID3D12InfoQueue1>(&infoQueue)))
            {
                infoQueue->RegisterMessageCallback(
                        DebugLayerMessageCallback,
                        D3D12_MESSAGE_CALLBACK_FLAG_NONE,
                        this,
                        &m_validationLayerMessageCallbackHandle);
            }
        }
#endif
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
#if !defined(KE_FINAL)
            Dx12SetName(m_directQueue.Get(), L"Direct queue");
#endif

        }

        if ((!features.m_graphics || features.m_asyncCompute) && features.m_compute )
        {
            D3D12_COMMAND_QUEUE_DESC computeQueueDesc {};
            computeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            computeQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            computeQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            computeQueueDesc.NodeMask = 0;

            Dx12Assert(m_device->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&m_computeQueue)));
#if !defined(KE_FINAL)
            Dx12SetName(m_computeQueue.Get(), L"Compute queue");
#endif
        }

        if ((!features.m_graphics && !features.m_compute || features.m_transferQueue) && features.m_transfer)
        {
            D3D12_COMMAND_QUEUE_DESC copyQueueDesc {};
            copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            copyQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            copyQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            copyQueueDesc.NodeMask = 0;

            Dx12Assert(m_device->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&m_copyQueue)));
#if !defined(KE_FINAL)
            Dx12SetName(m_copyQueue.Get(), L"Copy queue");
#endif
        }
    }
} // KryneEngine