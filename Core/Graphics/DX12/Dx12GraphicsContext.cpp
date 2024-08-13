/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#include "Dx12GraphicsContext.hpp"
#include "Dx12DescriptorSetManager.hpp"
#include "Dx12SwapChain.hpp"
#include "HelperFunctions.hpp"
#include <Common/Utils/Alignment.hpp>
#include <D3D12MemAlloc.h>
#include <Graphics/Common/Buffer.hpp>
#include <Graphics/Common/Drawing.hpp>
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

        m_resources.InitHeaps(m_device.Get());

        m_descriptorSetManager = eastl::make_unique<Dx12DescriptorSetManager>();
        m_descriptorSetManager->Init(m_device.Get(), m_frameContextCount, _currentFrameId % m_frameContextCount);

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
        m_descriptorSetManager->NextFrame(m_device.Get(), m_resources, nextFrameIndex);
    }

    bool Dx12GraphicsContext::IsFrameExecuted(KryneEngine::u64 _frameId) const
    {
        return m_frameFence->GetCompletedValue() >= _frameId;
    }

    void Dx12GraphicsContext::WaitForFrame(u64 _frameId) const
    {
        if (m_frameFence->GetCompletedValue() < _frameId)
        {
            Dx12Assert(m_frameFence->SetEventOnCompletion(_frameId, m_frameFenceEvent));
            WaitForSingleObject(m_frameFenceEvent, INFINITE);
        }
    }

    void Dx12GraphicsContext::_CreateDevice(IDXGIFactory4* _factory4)
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        _FindAdapter(_factory4, &hardwareAdapter);

        Dx12Assert(D3D12CreateDevice(hardwareAdapter.Get(),
                                     Dx12Converters::GetFeatureLevel(m_appInfo),
                                     IID_PPV_ARGS(&m_device)));
#if !defined(KE_FINAL)
        Dx12SetName(m_device.Get(), L"Device");
#endif

        m_resources.InitAllocator(m_device.Get(), hardwareAdapter.Get());

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

        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};
            Dx12Assert(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12)));
            m_enhancedBarriersEnabled = options12.EnhancedBarriersSupported;
        }
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

    SamplerHandle Dx12GraphicsContext::CreateSampler(const SamplerDesc& _samplerDesc)
    {
        return m_resources.CreateSampler(_samplerDesc, m_device.Get());
    }

    bool Dx12GraphicsContext::DestroySampler(SamplerHandle _sampler)
    {
        return m_resources.DestroySampler(_sampler);
    }

    RenderTargetViewHandle Dx12GraphicsContext::GetPresentRenderTargetView(u8 _index)
    {
        return m_swapChain->m_renderTargetViews[_index];
    }

    CommandList Dx12GraphicsContext::BeginGraphicsCommandList(u64 _frameId)
    {
        const u8 frameIndex = _frameId % m_frameContextCount;
        CommandList list = m_frameContexts[frameIndex].BeginDirectCommandList();
        m_descriptorSetManager->OnBeginGraphicsCommandList(list, frameIndex);
        return list;
    }

    void Dx12GraphicsContext::EndGraphicsCommandList(u64 _frameId)
    {
        m_frameContexts[_frameId % m_frameContextCount].EndDirectCommandList();
    }

    void Dx12GraphicsContext::BeginRenderPass(CommandList _commandList, RenderPassHandle _renderPass)
    {
        const auto* desc = m_resources.m_renderPasses.Get(_renderPass.m_handle);
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

            auto* rtvData = m_resources.m_renderTargetViews.Get(attachment.m_rtv.m_handle);
            VERIFY_OR_RETURN_VOID(rtvData != nullptr);

            colorAttachments.push_back(D3D12_RENDER_PASS_RENDER_TARGET_DESC { rtvData->m_cpuHandle, beginningAccess, endingAccess });

            addBarrier(attachment, *m_resources.m_textures.Get(rtvData->m_resource.m_handle));
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

            auto* rtvData = m_resources.m_renderTargetViews.Get(attachment.m_rtv.m_handle);
            VERIFY_OR_RETURN_VOID(rtvData != nullptr);

            depthStencilDesc = {
                    rtvData->m_cpuHandle,
                    depthBeginningAccess,
                    stencilBeginningAccess,
                    depthEndingAccess,
                    stencilEndingAccess
            };

            addBarrier(attachment, *m_resources.m_textures.Get(rtvData->m_resource.m_handle), true);
        }

        _commandList->ResourceBarrier(barriers.size(), barriers.data());

        _commandList->BeginRenderPass(
                colorAttachments.size(),
                colorAttachments.data(),
                desc->m_depthStencilAttachment.has_value() ? &depthStencilDesc : nullptr,
                D3D12_RENDER_PASS_FLAG_NONE);

        m_currentRenderPass = _renderPass;
    }

    void Dx12GraphicsContext::EndRenderPass(CommandList _commandList)
    {
        const auto* desc = m_resources.m_renderPasses.Get(m_currentRenderPass.m_handle);
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
            auto* rtvData = m_resources.m_renderTargetViews.Get(attachment.m_rtv.m_handle);
            VERIFY_OR_RETURN_VOID(rtvData != nullptr);

            addBarrier(attachment, *m_resources.m_textures.Get(rtvData->m_resource.m_handle));
        }

        if (desc->m_depthStencilAttachment.has_value())
        {
            const auto& attachment = desc->m_depthStencilAttachment.value();
            auto* rtvData = m_resources.m_renderTargetViews.Get(attachment.m_rtv.m_handle);
            VERIFY_OR_RETURN_VOID(rtvData != nullptr);

            addBarrier(attachment, *m_resources.m_textures.Get(rtvData->m_resource.m_handle), true);
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
        BufferHandle _stagingBuffer,
        TextureHandle _dstTexture,
        const TextureMemoryFootprint& _footprint,
        const SubResourceIndexing& _subResourceIndex,
        void* _data)
    {
        ID3D12Resource** stagingTexture = m_resources.m_buffers.Get(_stagingBuffer.m_handle);
        ID3D12Resource** dstTexture = m_resources.m_textures.Get(_dstTexture.m_handle);

        VERIFY_OR_RETURN_VOID(stagingTexture != nullptr);
        VERIFY_OR_RETURN_VOID(dstTexture != nullptr);

        const D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint {
            .Offset = _footprint.m_offset,
            .Footprint = {
                .Format = Dx12Converters::ToDx12Format(_footprint.m_format),
                .Width = _footprint.m_width,
                .Height = _footprint.m_height,
                .Depth = _footprint.m_depth,
                .RowPitch = _footprint.m_lineByteAlignedSize,
            },
        };

        const s64 inputRowPitch = footprint.Footprint.Width * GetTextureBytesPerPixel(footprint.Footprint.Format);
        const D3D12_SUBRESOURCE_DATA data {
            .pData = _data,
            .RowPitch = inputRowPitch,
            .SlicePitch = inputRowPitch * static_cast<s64>(footprint.Footprint.Height),
        };

        {
            void* bufferData;
            (*stagingTexture)->Map(0, nullptr, &bufferData);

            const D3D12_MEMCPY_DEST copyInfo {
                .pData = (u8*)bufferData + footprint.Offset,
                .RowPitch = footprint.Footprint.RowPitch,
                .SlicePitch = footprint.Footprint.RowPitch * footprint.Footprint.Height,
            };
            MemcpySubresource(
                &copyInfo,
                &data,
                footprint.Footprint.RowPitch,
                footprint.Footprint.Height,
                footprint.Footprint.Depth);

            (*stagingTexture)->Unmap(0, nullptr);
        }

        const u32 subResourceIndex = D3D12CalcSubresource(
            _subResourceIndex.m_mipIndex,
            _subResourceIndex.m_arraySlice,
            Dx12Converters::RetrievePlaneSlice(_subResourceIndex.m_planes, _subResourceIndex.m_planeSlice),
            _subResourceIndex.m_mipCount,
            _subResourceIndex.m_arraySize);

        const CD3DX12_TEXTURE_COPY_LOCATION Dst(*dstTexture, subResourceIndex);
        const CD3DX12_TEXTURE_COPY_LOCATION Src(*stagingTexture, footprint);
        _commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
    }

    eastl::vector<TextureMemoryFootprint> Dx12GraphicsContext::FetchTextureSubResourcesMemoryFootprints(const TextureDesc& _desc)
    {
        D3D12_RESOURCE_DESC resourceDesc {
            .Dimension = Dx12Converters::GetTextureResourceDimension(_desc.m_type),
            .Alignment = 0,
            .Width = _desc.m_dimensions.x,
            .Height = _desc.m_dimensions.y,
            .DepthOrArraySize = static_cast<u16>(_desc.m_type == TextureTypes::Single3D
                                                     ? _desc.m_dimensions.z
                                                     : _desc.m_arraySize),
            .MipLevels = _desc.m_mipCount,
            .Format = Dx12Converters::ToDx12Format(_desc.m_format),
            .SampleDesc = { .Count = 1, .Quality = 0 },
        };

        const u32 numSubResources = _desc.m_arraySize * _desc.m_mipCount;

        DynamicArray<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints;
        footprints.Resize(numSubResources);

        m_device->GetCopyableFootprints(&resourceDesc, 0, numSubResources, 0, footprints.Data(), nullptr, nullptr, nullptr);

        eastl::vector<TextureMemoryFootprint> finalFootprints;
        for (const auto& footprint: footprints)
        {
            finalFootprints.push_back(TextureMemoryFootprint {
                .m_offset = footprint.Offset,
                .m_width = footprint.Footprint.Width,
                .m_height = footprint.Footprint.Height,
                .m_lineByteAlignedSize = footprint.Footprint.RowPitch,
                .m_depth = static_cast<u16>(footprint.Footprint.Depth),
                .m_format = Dx12Converters::FromDx12Format(footprint.Footprint.Format),
            });
        }

        return finalFootprints;
    }

    bool Dx12GraphicsContext::NeedsStagingBuffer(BufferHandle _buffer)
    {
        D3D12MA::Allocation** pAllocation = m_resources.m_buffers.GetCold(_buffer.m_handle);
        VERIFY_OR_RETURN(pAllocation != nullptr, false);
        D3D12MA::Allocation* allocation = *pAllocation;

        return allocation->GetHeap()->GetDesc().Properties.Type != D3D12_HEAP_TYPE_UPLOAD;
    }

    void Dx12GraphicsContext::MapBuffer(BufferMapping& _mapping)
    {
        D3D12MA::Allocation** pAllocation = m_resources.m_buffers.GetCold(_mapping.m_buffer.m_handle);
        VERIFY_OR_RETURN_VOID(pAllocation != nullptr);
        D3D12MA::Allocation* allocation = *pAllocation;

        KE_ASSERT_MSG(_mapping.m_ptr == nullptr, "Structure still holds a mapping");
        KE_ASSERT(allocation->GetSize() >= _mapping.m_offset);
        KE_ASSERT(_mapping.m_size == ~0 || allocation->GetSize() >= _mapping.m_offset + _mapping.m_size);
        _mapping.m_size = eastl::min(_mapping.m_size, allocation->GetSize() - _mapping.m_offset);

        D3D12_RANGE range { 0, 0 };
        if (!_mapping.m_pureWrite)
        {
            range = { _mapping.m_offset, _mapping.m_offset + _mapping.m_size };
        }

        void* ptr;
        Dx12Assert(allocation->GetResource()->Map(0, &range, &ptr));
        _mapping.m_ptr = (u8*)ptr + _mapping.m_offset;
    }

    void Dx12GraphicsContext::UnmapBuffer(BufferMapping& _mapping)
    {
        ID3D12Resource** pBuffer = m_resources.m_buffers.Get(_mapping.m_buffer.m_handle);
        VERIFY_OR_RETURN_VOID(pBuffer != nullptr);
        KE_ASSERT_MSG(_mapping.m_ptr != nullptr, "Structure holds no mapping");

        const D3D12_RANGE range { _mapping.m_offset, _mapping.m_offset + _mapping.m_size };
        (*pBuffer)->Unmap(0, &range);
        _mapping.m_ptr = nullptr;
    }

    void Dx12GraphicsContext::CopyBuffer(CommandList _commandList, const BufferCopyParameters& _params)
    {
        ID3D12Resource** bufferSrc = m_resources.m_buffers.Get(_params.m_bufferSrc.m_handle);
        ID3D12Resource** bufferDst = m_resources.m_buffers.Get(_params.m_bufferDst.m_handle);
        VERIFY_OR_RETURN_VOID(bufferSrc != nullptr && bufferDst != nullptr);

        _commandList->CopyBufferRegion(
            *bufferDst,
            _params.m_offsetDst,
            *bufferSrc,
            _params.m_offsetSrc,
            _params.m_copySize);
    }

    void Dx12GraphicsContext::PlaceMemoryBarriers(
        CommandList _commandList,
        const eastl::span<GlobalMemoryBarrier>& _globalMemoryBarriers,
        const eastl::span<BufferMemoryBarrier>& _bufferMemoryBarriers,
        const eastl::span<TextureMemoryBarrier>& _textureMemoryBarriers)
    {
        using namespace Dx12Converters;

        if (m_enhancedBarriersEnabled)
        {
            eastl::fixed_vector<D3D12_BARRIER_GROUP, 3> barrierGroups;

            if (!_globalMemoryBarriers.empty())
            {
                DynamicArray<D3D12_GLOBAL_BARRIER> globalMemoryBarriers(_globalMemoryBarriers.size());

                for (auto i = 0u; i < globalMemoryBarriers.Size(); i++)
                {
                    const GlobalMemoryBarrier& barrier = _globalMemoryBarriers[i];

                    globalMemoryBarriers[i] = D3D12_GLOBAL_BARRIER{
                        .SyncBefore = ToDx12BarrierSync(barrier.m_stagesSrc),
                        .SyncAfter = ToDx12BarrierSync(barrier.m_stagesDst),
                        .AccessBefore = ToDx12BarrierAccess(barrier.m_accessSrc),
                        .AccessAfter = ToDx12BarrierAccess(barrier.m_accessDst),
                    };
                }

                barrierGroups.push_back(D3D12_BARRIER_GROUP{
                    .Type = D3D12_BARRIER_TYPE_GLOBAL,
                    .NumBarriers = (u32)globalMemoryBarriers.Size(),
                    .pGlobalBarriers = globalMemoryBarriers.Data(),
                });
            }

            if (!_bufferMemoryBarriers.empty())
            {
                DynamicArray<D3D12_BUFFER_BARRIER> bufferMemoryBarriers(_bufferMemoryBarriers.size());

                for (auto i = 0u; i < bufferMemoryBarriers.Size(); i++)
                {
                    const BufferMemoryBarrier& barrier = _bufferMemoryBarriers[i];
                    ID3D12Resource** buffer = m_resources.m_buffers.Get(barrier.m_buffer.m_handle);

                    bufferMemoryBarriers[i] = D3D12_BUFFER_BARRIER{
                        .SyncBefore = ToDx12BarrierSync(barrier.m_stagesSrc),
                        .SyncAfter = ToDx12BarrierSync(barrier.m_stagesDst),
                        .AccessBefore = ToDx12BarrierAccess(barrier.m_accessSrc),
                        .AccessAfter = ToDx12BarrierAccess(barrier.m_accessDst),
                        .pResource = buffer != nullptr ? *buffer : nullptr,
                        .Offset = barrier.m_offset,
                        .Size = barrier.m_size,
                    };
                }

                barrierGroups.push_back(D3D12_BARRIER_GROUP{
                    .Type = D3D12_BARRIER_TYPE_BUFFER,
                    .NumBarriers = (u32)bufferMemoryBarriers.Size(),
                    .pBufferBarriers = bufferMemoryBarriers.Data(),
                });
            }

            if (!_textureMemoryBarriers.empty())
            {
                DynamicArray<D3D12_TEXTURE_BARRIER> textureMemoryBarriers(_textureMemoryBarriers.size());

                for (auto i = 0u; i < textureMemoryBarriers.Size(); i++)
                {
                    const TextureMemoryBarrier& barrier = _textureMemoryBarriers[i];
                    ID3D12Resource** texture = m_resources.m_textures.Get(barrier.m_texture.m_handle);

                    textureMemoryBarriers[i] = D3D12_TEXTURE_BARRIER{
                        .SyncBefore = ToDx12BarrierSync(barrier.m_stagesSrc),
                        .SyncAfter = ToDx12BarrierSync(barrier.m_stagesDst),
                        .AccessBefore = ToDx12BarrierAccess(barrier.m_accessSrc),
                        .AccessAfter = ToDx12BarrierAccess(barrier.m_accessDst),
                        .LayoutBefore = ToDx12BarrierLayout(barrier.m_layoutSrc),
                        .LayoutAfter = ToDx12BarrierLayout(barrier.m_layoutDst),
                        .pResource = texture == nullptr ? nullptr : *texture,
                        .Subresources =
                            {.IndexOrFirstMipLevel = barrier.m_mipStart,
                             .NumMipLevels = barrier.m_mipCount,
                             .FirstArraySlice = barrier.m_arrayStart,
                             .NumArraySlices = barrier.m_arrayCount,
                             .FirstPlane = 0,
                             .NumPlanes = (u32)std::popcount((u8)barrier.m_planes)},
                        .Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE,
                    };
                }

                barrierGroups.push_back(D3D12_BARRIER_GROUP{
                    .Type = D3D12_BARRIER_TYPE_TEXTURE,
                    .NumBarriers = (u32)textureMemoryBarriers.Size(),
                    .pTextureBarriers = textureMemoryBarriers.Data(),
                });
            }

            _commandList->Barrier(barrierGroups.size(), barrierGroups.data());
        }
        else
        {
            eastl::vector<D3D12_RESOURCE_BARRIER> resourceBarriers {};

            for (const auto& barrier: _textureMemoryBarriers)
            {
                ID3D12Resource** texture = m_resources.m_textures.Get(barrier.m_texture.m_handle);
                if (texture == nullptr)
                {
                    continue;
                }

                D3D12_RESOURCE_STATES before = RetrieveState(barrier.m_accessSrc, barrier.m_layoutSrc);
                D3D12_RESOURCE_STATES after = RetrieveState(barrier.m_accessDst, barrier.m_layoutDst);

                for (u8 mip = barrier.m_mipStart; mip < barrier.m_mipCount; mip++)
                {
                    for (u16 slice = barrier.m_arrayStart; slice < barrier.m_arrayCount; slice++)
                    {
                        u32 subResourceIndex = D3D12CalcSubresource(mip, slice, 0, barrier.m_mipCount, barrier.m_arrayCount);

                        resourceBarriers.push_back(D3D12_RESOURCE_BARRIER {
                            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                            .Transition = {
                                .pResource = *texture,
                                .Subresource = subResourceIndex,
                                .StateBefore = before,
                                .StateAfter = after,
                            }
                        });
                    }
                }

                if (BitUtils::EnumHasAny(barrier.m_accessSrc, BarrierAccessFlags::UnorderedAccess)
                    && BitUtils::EnumHasAny(barrier.m_accessDst, BarrierAccessFlags::UnorderedAccess))
                {
                    resourceBarriers.push_back({
                        .Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
                        .UAV = { .pResource = *texture },
                    });
                }
            }

            for (const auto& barrier: _bufferMemoryBarriers)
            {
                ID3D12Resource** buffer = m_resources.m_buffers.Get(barrier.m_buffer.m_handle);
                if (buffer == nullptr)
                {
                    continue;
                }

                D3D12_RESOURCE_STATES before = RetrieveState(barrier.m_accessSrc, TextureLayout::Common);
                D3D12_RESOURCE_STATES after = RetrieveState(barrier.m_accessDst, TextureLayout::Common);

                resourceBarriers.push_back(D3D12_RESOURCE_BARRIER {
                    .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                    .Transition = {
                        .pResource = *buffer,
                        .Subresource = 0,
                        .StateBefore = before,
                        .StateAfter = after,
                    }
                });

                if (BitUtils::EnumHasAny(barrier.m_accessSrc, BarrierAccessFlags::UnorderedAccess)
                    && BitUtils::EnumHasAny(barrier.m_accessDst, BarrierAccessFlags::UnorderedAccess))
                {
                    resourceBarriers.push_back({
                        .Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
                        .UAV = { .pResource = *buffer},
                    });
                }
            }

            for (const auto& barrier: _globalMemoryBarriers)
            {
                if (KE_VERIFY_MSG(BitUtils::EnumHasAny(barrier.m_accessSrc, BarrierAccessFlags::UnorderedAccess)
                                  && BitUtils::EnumHasAny(barrier.m_accessDst, BarrierAccessFlags::UnorderedAccess),
                                  "Global memory barriers for anything other than UAV barriers are not supported without enhanced barriers."))
                {
                    resourceBarriers.push_back({
                        .Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
                        .UAV = { .pResource = nullptr },
                    });
                }
            }

            _commandList->ResourceBarrier(resourceBarriers.size(), resourceBarriers.data());
        }
    }

    ShaderModuleHandle Dx12GraphicsContext::RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize)
    {
        return m_resources.RegisterShaderModule(_bytecodeData, _bytecodeSize);
    }

    DescriptorSetHandle Dx12GraphicsContext::CreateDescriptorSet(const DescriptorSetDesc& _desc, u32* _bindingIndices)
    {
        return m_descriptorSetManager->CreateDescriptorSet(_desc, _bindingIndices);
    }

    PipelineLayoutHandle Dx12GraphicsContext::CreatePipelineLayout(const PipelineLayoutDesc& _desc)
    {
        return m_resources.CreatePipelineLayout(_desc, m_device.Get());
    }

    GraphicsPipelineHandle Dx12GraphicsContext::CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc)
    {
        return m_resources.CreateGraphicsPipeline(_desc, m_device.Get());
    }

    void Dx12GraphicsContext::UpdateDescriptorSet(
        DescriptorSetHandle _descriptorSet,
        const eastl::span<DescriptorSetWriteInfo>& _writes,
        u64 _frameId)
    {
        m_descriptorSetManager->UpdateDescriptorSet(
            _descriptorSet,
            m_resources, _writes,
            m_device.Get(),
            _frameId % m_frameContextCount);
    }

    void Dx12GraphicsContext::SetViewport(CommandList _commandList, const Viewport& _viewport)
    {
        const D3D12_VIEWPORT viewport {
            .TopLeftX = static_cast<float>(_viewport.m_topLeftX),
            .TopLeftY = static_cast<float>(_viewport.m_topLeftY),
            .Width = static_cast<float>(_viewport.m_width),
            .Height = static_cast<float>(_viewport.m_height),
            .MinDepth = _viewport.m_minDepth,
            .MaxDepth = _viewport.m_maxDepth,
        };
        _commandList->RSSetViewports(1, &viewport);
    }

    void Dx12GraphicsContext::SetScissorsRect(CommandList _commandList, const Rect& _rect)
    {
        const D3D12_RECT scissorRect = {
            .left = static_cast<LONG>(_rect.m_left),
            .top = static_cast<LONG>(_rect.m_top),
            .right = static_cast<LONG>(_rect.m_right),
            .bottom = static_cast<LONG>(_rect.m_bottom),
        };
        _commandList->RSSetScissorRects(1, &scissorRect);
    }

    void Dx12GraphicsContext::SetIndexBuffer(CommandList _commandList, const BufferView& _indexBufferView, bool _isU16)
    {
        VERIFY_OR_RETURN_VOID(_indexBufferView.m_buffer != GenPool::kInvalidHandle);
        ID3D12Resource** pIndexBuffer = m_resources.m_buffers.Get(_indexBufferView.m_buffer.m_handle);
        VERIFY_OR_RETURN_VOID(pIndexBuffer != nullptr);

        const D3D12_INDEX_BUFFER_VIEW indexBufferView {
            .BufferLocation = (*pIndexBuffer)->GetGPUVirtualAddress() + _indexBufferView.m_offset,
            .SizeInBytes = static_cast<u32>(_indexBufferView.m_size),
            .Format = _isU16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
        };

        _commandList->IASetIndexBuffer(&indexBufferView);
    }

    void Dx12GraphicsContext::SetVertexBuffers(CommandList _commandList, const eastl::span<BufferView>& _bufferViews)
    {
        eastl::fixed_vector<D3D12_VERTEX_BUFFER_VIEW, 4> bufferViews;
        bufferViews.reserve(_bufferViews.size());

        for (const auto& view: _bufferViews)
        {
            VERIFY_OR_RETURN_VOID(view.m_buffer != GenPool::kInvalidHandle);
            ID3D12Resource** pIndexBuffer = m_resources.m_buffers.Get(view.m_buffer.m_handle);
            VERIFY_OR_RETURN_VOID(pIndexBuffer != nullptr);

            bufferViews.push_back({
                .BufferLocation = (*pIndexBuffer)->GetGPUVirtualAddress() + view.m_offset,
                .SizeInBytes = static_cast<u32>(view.m_size),
                .StrideInBytes = view.m_stride,
            });
        }

        _commandList->IASetVertexBuffers(0, bufferViews.size(), bufferViews.data());
    }

    void Dx12GraphicsContext::SetGraphicsPipeline(CommandList _commandList, GraphicsPipelineHandle _graphicsPipeline)
    {
        VERIFY_OR_RETURN_VOID(_graphicsPipeline != GenPool::kInvalidHandle);
        ID3D12PipelineState** pPso = m_resources.m_pipelineStateObjects.Get(_graphicsPipeline.m_handle);
        VERIFY_OR_RETURN_VOID(pPso != nullptr);
        Dx12Resources::PsoColdData* coldData = m_resources.m_pipelineStateObjects.GetCold(_graphicsPipeline.m_handle);
        VERIFY_OR_RETURN_VOID(coldData != nullptr);

        _commandList->SetGraphicsRootSignature(coldData->m_signature);
        _commandList->IASetPrimitiveTopology(Dx12Converters::ToDx12Topology(coldData->m_topology));
        _commandList->SetPipelineState(*pPso);
    }

    void Dx12GraphicsContext::SetGraphicsPushConstant(
        CommandList _commandList,
        u32 _index,
        const eastl::span<u32>& _data,
        u32 _offset)
    {
        _commandList->SetGraphicsRoot32BitConstants(
            _index,
            _data.size(),
            _data.data(),
            _offset);
    }

    void Dx12GraphicsContext::SetGraphicsDescriptorSets(
        CommandList _commandList, const eastl::span<DescriptorSetHandle>& _sets, const bool* _unchanged, u32 _frameId)
    {
        m_descriptorSetManager->SetGraphicsDescriptorSets(
            _commandList,
            _sets,
            _unchanged,
            _frameId % m_frameContextCount);
    }

    void Dx12GraphicsContext::DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc)
    {
        _commandList->DrawIndexedInstanced(
            _desc.m_elementCount,
            _desc.m_instanceCount,
            _desc.m_indexOffset,
            _desc.m_vertexOffset,
            _desc.m_instanceOffset);
    }
} // KryneEngine