/**
 * @file
 * @author Max Godefroy
 * @date 23/02/2024.
 */

#include "Dx12Resources.h"
#include "HelperFunctions.hpp"
#include <D3D12MemAlloc.h>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/Common/ResourceViews/RenderTargetView.hpp>
#include <Graphics/Common/ResourceViews/ShaderResourceView.hpp>
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    Dx12Resources::Dx12Resources() = default;
    Dx12Resources::~Dx12Resources() = default;

    void Dx12Resources::Init(ID3D12Device* _device, IDXGIAdapter* _adapter)
    {
        D3D12MA::ALLOCATOR_DESC allocatorDesc {
            .pDevice = _device,
            .pAdapter = _adapter,
        };

        Dx12Assert(D3D12MA::CreateAllocator(&allocatorDesc, &m_memoryAllocator));
    }

    GenPool::Handle Dx12Resources::CreateTexture(const TextureCreateDesc& _createDesc, ID3D12Device* _device)
    {
        D3D12_RESOURCE_DESC resourceDesc {
            .Dimension = Dx12Converters::GetTextureResourceDimension(_createDesc.m_desc.m_type),
            .Alignment = 0,
            .Width = _createDesc.m_desc.m_dimensions.x,
            .Height = _createDesc.m_desc.m_dimensions.y,
            .DepthOrArraySize = static_cast<u16>(_createDesc.m_desc.m_type == TextureTypes::Single3D
                                    ? _createDesc.m_desc.m_dimensions.z
                                    : _createDesc.m_desc.m_arraySize),
            .MipLevels = _createDesc.m_desc.m_mipCount,
            .Format = Dx12Converters::ToDx12Format(_createDesc.m_desc.m_format),
            .SampleDesc = { .Count = 1, .Quality = 0 },
        };

        const D3D12MA::ALLOCATION_DESC allocationDesc {
            .HeapType = Dx12Converters::GetHeapType(_createDesc.m_memoryUsage),
        };

        if (allocationDesc.HeapType == D3D12_HEAP_TYPE_UPLOAD)
        {
            u64 bufferWidth = 0;
            _device->GetCopyableFootprints(
                &resourceDesc,
                0,
                _createDesc.m_desc.m_mipCount * _createDesc.m_desc.m_arraySize,
                0,
                nullptr,
                nullptr,
                nullptr,
                &bufferWidth);

            resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
                bufferWidth,
                resourceDesc.Flags,
                resourceDesc.Alignment);
        }

        D3D12MA::Allocation* allocation;
        ID3D12Resource* texture;
        Dx12Assert(m_memoryAllocator->CreateResource(
            &allocationDesc,
            &resourceDesc, D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            &allocation,
            IID_PPV_ARGS(&texture)));

        return RegisterTexture(texture, allocation);
    }

    GenPool::Handle Dx12Resources::RegisterTexture(ID3D12Resource* _texture, D3D12MA::Allocation* _allocation)
    {
        const auto handle = m_textures.Allocate();
        *m_textures.Get(handle) = _texture;
        *m_textures.GetCold(handle) = _allocation;
        return handle;
    }

    bool Dx12Resources::ReleaseTexture(GenPool::Handle _handle, bool _free)
    {
        ID3D12Resource* texture = nullptr;
        D3D12MA::Allocation* allocation = nullptr;
        if (m_textures.Free(_handle, _free ? &texture : nullptr, &allocation))
        {
            SafeRelease(texture);

            if (allocation != nullptr)
            {
                allocation->Release();
            }

            return true;
        }
        return false;
    }

    GenPool::Handle Dx12Resources::CreateRenderTargetView(const RenderTargetViewDesc &_desc, ID3D12Device *_device)
    {
        auto* texture = m_textures.Get(_desc.m_textureHandle);
        if (texture == nullptr)
        {
            return GenPool::kInvalidHandle;
        }

        const auto handle = m_renderTargetViews.Allocate();
        KE_ASSERT_FATAL_MSG(handle.m_index < kRtvHeapSize, "RTV heap only supports up to %d concurrent descriptors. Try to improve architecture, or increase Dx12Resources::kRtvHeapSize");

        if (m_rtvDescriptorHeap == nullptr)
        {
            const D3D12_DESCRIPTOR_HEAP_DESC heapDesc {
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                .NumDescriptors = kRtvHeapSize,
                .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE // Not shader visible
            };
            Dx12Assert(_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap)));
#if !defined(KE_FINAL)
            Dx12SetName(m_rtvDescriptorHeap.Get(), L"RTV descriptor heap");
#endif
            m_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc {
            .Format = Dx12Converters::ToDx12Format(_desc.m_format)
        };

        switch (_desc.m_type)
        {
            case TextureTypes::Single1D:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                rtvDesc.Texture1D.MipSlice = _desc.m_mipLevel;
                break;
            case TextureTypes::Single2D:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = _desc.m_mipLevel;
                rtvDesc.Texture2D.PlaneSlice = 0;
                break;
            case TextureTypes::Single3D:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                rtvDesc.Texture3D.MipSlice = _desc.m_mipLevel;
                rtvDesc.Texture3D.FirstWSlice = _desc.m_depthStartSlice;
                rtvDesc.Texture3D.WSize = _desc.m_depthSlicesSize;
                break;
            case TextureTypes::Array1D:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                rtvDesc.Texture1DArray.MipSlice = _desc.m_mipLevel;
                rtvDesc.Texture1DArray.FirstArraySlice = _desc.m_arrayRangeStart;
                rtvDesc.Texture1DArray.ArraySize = _desc.m_arrayRangeSize;
                break;
            case TextureTypes::Array2D:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.MipSlice = _desc.m_mipLevel;
                rtvDesc.Texture2DArray.FirstArraySlice = _desc.m_arrayRangeStart;
                rtvDesc.Texture2DArray.ArraySize = _desc.m_arrayRangeSize;
                rtvDesc.Texture2DArray.PlaneSlice = 0;
                break;
            case TextureTypes::SingleCube:
            case TextureTypes::ArrayCube:
                KE_FATAL("Invalid RTV texture type");
        }

        const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle(
                m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                handle.m_index,
                m_rtvDescriptorSize);
        _device->CreateRenderTargetView(*texture, &rtvDesc, cpuDescriptorHandle);

        *m_renderTargetViews.Get(handle) = RtvHotData {
            .m_cpuHandle = cpuDescriptorHandle,
            .m_resource = _desc.m_textureHandle,
        };

        return handle;
    }

    bool Dx12Resources::FreeRenderTargetView(GenPool::Handle _handle)
    {
        // Don't have to destroy anything, as the memory slot will be marked as free.
        // Only the heap itself will need to be freed using API.
        return m_renderTargetViews.Free(_handle);
    }

    GenPool::Handle Dx12Resources::CreateRenderPass(const RenderPassDesc &_desc)
    {
        auto handle = m_renderPasses.Allocate();
        auto* desc = m_renderPasses.Get(handle);

        // Manually init pointer location using a copy, as the allocator doesn't initialize its objects.
        new (desc) RenderPassDesc(_desc);
        return handle;
    }

    bool Dx12Resources::FreeRenderPass(GenPool::Handle _handle)
    {
        // Simply mark slot as available.
        return m_renderPasses.Free(_handle);
    }

    GenPool::Handle Dx12Resources::CreateTextureSrv(
        const TextureSrvDesc& _srvDesc,
        ID3D12Device* _device, u32 _frameContextCount,
        u32 _frameIndex)
    {
        auto* texturePtr = m_textures.Get(_srvDesc.m_textureHandle);
        VERIFY_OR_RETURN(texturePtr != nullptr, GenPool::kInvalidHandle);
        ID3D12Resource* texture = *texturePtr;

        const GenPool::Handle handle = m_cbvSrvUav.Allocate();

        if (m_cbvSrvUavDescriptorHeaps.Empty())
        {
            m_cbvSrvUavDescriptorHeaps.Resize(_frameContextCount);
            m_cbvSrvUavDescriptorHeaps.InitAll(nullptr);

            m_cbvSrvUavDescriptorCopyTracker.Init(_frameContextCount, _frameIndex);

            for (u32 i = 0; i < _frameContextCount; i++)
            {{
                    const D3D12_DESCRIPTOR_HEAP_DESC heapDesc {
                        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                        .NumDescriptors = kCbvSrvUavHeapSize,
                        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                    };
                    Dx12Assert(_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvSrvUavDescriptorHeaps[i])));
#if !defined(KE_FINAL)
                    Dx12SetName(m_cbvSrvUavDescriptorHeaps[i].Get(), L"CBV/SRV/UAV Descriptor Heap");
#endif
                    m_cbvSrvUavDescriptorSize = _device->GetDescriptorHandleIncrementSize(heapDesc.Type);
                }}
        }

        static_assert(static_cast<u8>(TextureComponentMapping::Red)   == D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0);
        static_assert(static_cast<u8>(TextureComponentMapping::Green) == D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1);
        static_assert(static_cast<u8>(TextureComponentMapping::Blue)  == D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2);
        static_assert(static_cast<u8>(TextureComponentMapping::Alpha) == D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3);
        static_assert(static_cast<u8>(TextureComponentMapping::Zero)  == D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0);
        static_assert(static_cast<u8>(TextureComponentMapping::One)   == D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {
            .Format = Dx12Converters::ToDx12Format(_srvDesc.m_format),
            .Shader4ComponentMapping = (u32)D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
                static_cast<u8>(_srvDesc.m_componentsMapping[0]),
                static_cast<u8>(_srvDesc.m_componentsMapping[1]),
                static_cast<u8>(_srvDesc.m_componentsMapping[2]),
                static_cast<u8>(_srvDesc.m_componentsMapping[3]))
        };

        const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle(
            m_cbvSrvUavDescriptorHeaps[_frameIndex]->GetCPUDescriptorHandleForHeapStart(),
            handle.m_index,
            m_cbvSrvUavDescriptorSize);
        _device->CreateShaderResourceView(texture, &srvDesc, cpuDescriptorHandle);

        *m_cbvSrvUav.Get(handle) = cpuDescriptorHandle;

        // Plan copy operations to spread descriptor to all frames
        m_cbvSrvUavDescriptorCopyTracker.TrackForOtherFrames(handle);

        return GenPool::kInvalidHandle;
    }

    void Dx12Resources::NextFrame(ID3D12Device* _device, u8 _frameIndex)
    {
        // Multi-frame descriptor creation
        if (!m_cbvSrvUavDescriptorHeaps.Empty())
        {
            const u8 nextFrame = (_frameIndex + 1) % m_cbvSrvUavDescriptorHeaps.Size();

            if (!m_cbvSrvUavDescriptorCopyTracker.GetData().empty())
            {
                eastl::vector<D3D12_CPU_DESCRIPTOR_HANDLE> srcHandles;
                eastl::vector<D3D12_CPU_DESCRIPTOR_HANDLE> dstHandles;
                eastl::vector<u32> counts;

                const auto& data = m_cbvSrvUavDescriptorCopyTracker.GetData();

                const auto srcHeapStart = m_cbvSrvUavDescriptorHeaps[_frameIndex]->GetCPUDescriptorHandleForHeapStart();
                const auto dstHeapStart = m_cbvSrvUavDescriptorHeaps[nextFrame]->GetCPUDescriptorHandleForHeapStart();

                srcHandles.reserve(data.size());
                dstHandles.reserve(data.size());
                counts.resize(data.size(), 1);

                for (GenPool::Handle handle : data)
                {
                    auto* cpuHandle = m_cbvSrvUav.Get(handle);
                    if (cpuHandle != nullptr)
                    {
                        srcHandles.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(
                            srcHeapStart,
                            handle.m_index,
                            m_cbvSrvUavDescriptorSize));
                        dstHandles.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(
                            dstHeapStart,
                            handle.m_index,
                            m_cbvSrvUavDescriptorSize));
                    }
                }

                _device->CopyDescriptors(
                    dstHandles.size(),
                    dstHandles.data(),
                    counts.data(),
                    srcHandles.size(),
                    srcHandles.data(),
                    counts.data(),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }

            m_cbvSrvUavDescriptorCopyTracker.AdvanceToNextFrame();
        }
    }
} // KryneEngine