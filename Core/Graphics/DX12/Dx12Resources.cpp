/**
 * @file
 * @author Max Godefroy
 * @date 23/02/2024.
 */

#include "Dx12Resources.h"
#include "HelperFunctions.hpp"
#include <D3D12MemAlloc.h>
#include <Graphics/Common/Texture.hpp>
#include <Graphics/Common/RenderTargetView.hpp>
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
        const D3D12_RESOURCE_DESC resourceDesc {
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
} // KryneEngine