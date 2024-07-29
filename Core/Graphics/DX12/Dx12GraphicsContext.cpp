/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#include "Dx12GraphicsContext.hpp"
#include "Dx12SwapChain.hpp"
#include "HelperFunctions.hpp"
#include <Common/Utils/Alignment.hpp>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/Common/Window.hpp>
#include <Memory/GenerationalPool.inl>
#include <dxgidebug.h>

namespace KryneEngine
{
    Dx12GraphicsContext::Dx12GraphicsContext(
            const GraphicsCommon::ApplicationInfo &_appInfo,
            u64 _currentFrameId)
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
                                                            m_directQueue.Get(),
                                                            m_resources);

            m_frameContextCount = m_swapChain->m_renderTargetViews.Size();
        }
        else
        {
            // If no display, remain on double buffering.
            m_frameContextCount = 2;
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
    }

    Dx12GraphicsContext::~Dx12GraphicsContext()
    {
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

        m_swapChain->Destroy(m_resources);
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
        const u8 frameIndex = _frameId % m_frameContextCount;

        auto& frameContext = m_frameContexts[frameIndex];

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
        frameContext.m_frameId = _frameId;
        frameContext.m_directCommandAllocationSet.Reset();
        frameContext.m_computeCommandAllocationSet.Reset();
        frameContext.m_copyCommandAllocationSet.Reset();

        // Retrieve next frame index
        const u8 nextFrameIndex = (_frameId + 1) % m_frameContextCount;

        // Wait for the previous frame with this index.
        WaitForFrame(m_frameContexts[nextFrameIndex].m_frameId);

        // Duplicate descriptor in multi-frame heaps
        m_resources.NextFrame(m_device.Get(), nextFrameIndex);
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

        m_resources.Init(m_device.Get(), hardwareAdapter.Get());

#if !defined(KE_FINAL)
        if (m_appInfo.m_features.m_validationLayers)
        {
            ComPtr<ID3D12InfoQueue1> infoQueue;
            if (SUCCEEDED(m_device->QueryInterface<ID3D12InfoQueue1>(&infoQueue)))
            {
                Dx12Assert(infoQueue->RegisterMessageCallback(
                        DebugLayerMessageCallback,
                        D3D12_MESSAGE_CALLBACK_FLAG_NONE,
                        this,
                        &m_validationLayerMessageCallbackHandle));
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

    GenPool::Handle Dx12GraphicsContext::GetPresentRenderTarget(u8 _index)
    {
        return m_swapChain->m_renderTargetTextures[_index];
    }

    CommandList Dx12GraphicsContext::BeginGraphicsCommandList(u64 _frameId)
    {
        return m_frameContexts[_frameId % m_frameContextCount].BeginDirectCommandList();
    }

    void Dx12GraphicsContext::EndGraphicsCommandList(u64 _frameId)
    {
        m_frameContexts[_frameId % m_frameContextCount].EndDirectCommandList();
    }

    void Dx12GraphicsContext::BeginRenderPass(CommandList _commandList, GenPool::Handle _handle)
    {
        const auto* desc = m_resources.m_renderPasses.Get(_handle);
        VERIFY_OR_RETURN_VOID(desc != nullptr);

        constexpr auto convertLoadOperation = [](RenderPassDesc::Attachment::LoadOperation _op)
        {
            switch (_op)
            {
                case RenderPassDesc::Attachment::LoadOperation::Load:
                    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                case RenderPassDesc::Attachment::LoadOperation::Clear:
                    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                case RenderPassDesc::Attachment::LoadOperation::DontCare:
                    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
            }
            KE_ERROR("Unreachable code");
            return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
        };

        constexpr auto convertStoreOperation = [](RenderPassDesc::Attachment::StoreOperation _op)
        {
            switch (_op)
            {
                case RenderPassDesc::Attachment::StoreOperation::Store:
                    return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                case RenderPassDesc::Attachment::StoreOperation::DontCare:
                    return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                case RenderPassDesc::Attachment::StoreOperation::Resolve:
                    return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
            }
            KE_ERROR("Unreachable code");
            return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
        };

        eastl::fixed_vector<D3D12_RESOURCE_BARRIER, RenderPassDesc::kMaxSupportedColorAttachments + 1, false> barriers;
        const auto addBarrier = [&barriers](const RenderPassDesc::Attachment& _desc, ID3D12Resource* _resource, bool _isDepthTarget = false)
        {
            const auto oldState = Dx12Converters::ToDx12ResourceState(_desc.m_initialLayout);
            const auto newState = _isDepthTarget ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_RENDER_TARGET;

            if (newState != oldState)
            {
                barriers.push_back(D3D12_RESOURCE_BARRIER{
                        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                        .Transition = D3D12_RESOURCE_TRANSITION_BARRIER{
                                .pResource = _resource,
                                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                                .StateBefore = oldState,
                                .StateAfter = newState,
                        }
                });
            }
        };

        eastl::fixed_vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC, RenderPassDesc::kMaxSupportedColorAttachments, false> colorAttachments;
        for (const auto& attachment: desc->m_colorAttachments)
        {
            CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_R32G32B32A32_FLOAT,  &attachment.m_clearColor[0]);

            D3D12_RENDER_PASS_BEGINNING_ACCESS beginningAccess {
                convertLoadOperation(attachment.m_loadOperation),
                { clearValue }
            };

            D3D12_RENDER_PASS_ENDING_ACCESS endingAccess {
                convertStoreOperation(attachment.m_storeOperation)
            };

            auto* rtvData = m_resources.m_renderTargetViews.Get(attachment.m_rtv);
            VERIFY_OR_RETURN_VOID(rtvData != nullptr);

            colorAttachments.push_back(D3D12_RENDER_PASS_RENDER_TARGET_DESC { rtvData->m_cpuHandle, beginningAccess, endingAccess });

            addBarrier(attachment, *m_resources.m_textures.Get(rtvData->m_resource));
        }

        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStencilDesc;
        if (desc->m_depthStencilAttachment.has_value())
        {
            const auto& attachment = desc->m_depthStencilAttachment.value();

            CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, attachment.m_clearColor[0], attachment.m_stencilClearValue);

            D3D12_RENDER_PASS_BEGINNING_ACCESS depthBeginningAccess {
                    convertLoadOperation(attachment.m_loadOperation),
                    { clearValue }
            };
            D3D12_RENDER_PASS_ENDING_ACCESS depthEndingAccess {
                convertStoreOperation(attachment.m_storeOperation)
            };

            D3D12_RENDER_PASS_BEGINNING_ACCESS stencilBeginningAccess {
                    convertLoadOperation(attachment.m_stencilLoadOperation),
                    { clearValue }
            };
            D3D12_RENDER_PASS_ENDING_ACCESS stencilEndingAccess {
                convertStoreOperation(attachment.m_stencilStoreOperation)
            };

            auto* rtvData = m_resources.m_renderTargetViews.Get(attachment.m_rtv);
            VERIFY_OR_RETURN_VOID(rtvData != nullptr);

            depthStencilDesc = {
                    rtvData->m_cpuHandle,
                    depthBeginningAccess,
                    stencilBeginningAccess,
                    depthEndingAccess,
                    stencilEndingAccess
            };

            addBarrier(attachment, *m_resources.m_textures.Get(rtvData->m_resource), true);
        }

        _commandList->ResourceBarrier(barriers.size(), barriers.data());

        _commandList->BeginRenderPass(
                colorAttachments.size(),
                colorAttachments.data(),
                desc->m_depthStencilAttachment.has_value() ? &depthStencilDesc : nullptr,
                D3D12_RENDER_PASS_FLAG_NONE);

        m_currentRenderPass = _handle;
    }

    void Dx12GraphicsContext::EndRenderPass(CommandList _commandList)
    {
        const auto* desc = m_resources.m_renderPasses.Get(m_currentRenderPass);
        VERIFY_OR_RETURN_VOID(desc != nullptr);

        _commandList->EndRenderPass();

        eastl::fixed_vector<D3D12_RESOURCE_BARRIER, RenderPassDesc::kMaxSupportedColorAttachments + 1, false> barriers;
        const auto addBarrier = [&barriers](const RenderPassDesc::Attachment& _desc, ID3D12Resource* _resource, bool _isDepthTarget = false)
        {
            const auto oldState = _isDepthTarget ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_RENDER_TARGET;
            const auto newState = Dx12Converters::ToDx12ResourceState(_desc.m_finalLayout);

            if (newState != oldState)
            {
                barriers.push_back(D3D12_RESOURCE_BARRIER{
                        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                        .Transition = D3D12_RESOURCE_TRANSITION_BARRIER{
                                .pResource = _resource,
                                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                                .StateBefore = oldState,
                                .StateAfter = newState,
                        }
                });
            }
        };

        for (const auto& attachment: desc->m_colorAttachments)
        {
            auto* rtvData = m_resources.m_renderTargetViews.Get(attachment.m_rtv);
            VERIFY_OR_RETURN_VOID(rtvData != nullptr);

            addBarrier(attachment, *m_resources.m_textures.Get(rtvData->m_resource));
        }

        if (desc->m_depthStencilAttachment.has_value())
        {
            const auto& attachment = desc->m_depthStencilAttachment.value();
            auto* rtvData = m_resources.m_renderTargetViews.Get(attachment.m_rtv);
            VERIFY_OR_RETURN_VOID(rtvData != nullptr);

            addBarrier(attachment, *m_resources.m_textures.Get(rtvData->m_resource), true);
        }

        _commandList->ResourceBarrier(barriers.size(), barriers.data());

        m_currentRenderPass = GenPool::kInvalidHandle;
    }

    u32 Dx12GraphicsContext::GetCurrentPresentImageIndex() const
    {
        return m_swapChain->GetBackBufferIndex();
    }

    void Dx12GraphicsContext::SetTextureData(
        CommandList _commandList,
        GenPool::Handle _stagingTexture,
        GenPool::Handle _dstTexture,
        const TextureDesc& _textureDesc,
        u8 _mipIndex,
        u16 _sliceIndex,
        void* _data)
    {
        D3D12_SUBRESOURCE_FOOTPRINT footprint {
            .Format = Dx12Converters::ToDx12Format(_textureDesc.m_format),
            .Width = _textureDesc.m_dimensions.x,
            .Height = _textureDesc.m_dimensions.y,
            .Depth = _textureDesc.m_dimensions.z,
        };

        footprint.RowPitch = Alignment::AlignUp<u32>(footprint.Width * 0, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    }
} // KryneEngine