/**
 * @file
 * @author Max Godefroy
 * @date 23/02/2024.
 */

#include "Dx12Resources.h"
#include "HelperFunctions.hpp"
#include "Dx12DescriptorSetManager.hpp"
#include <D3D12MemAlloc.h>
#include <Graphics/Common/Buffer.hpp>
#include <Graphics/Common/ResourceViews/RenderTargetView.hpp>
#include <Graphics/Common/ResourceViews/ShaderResourceView.hpp>
#include <Graphics/Common/ShaderPipeline.hpp>
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    Dx12Resources::Dx12Resources() = default;
    Dx12Resources::~Dx12Resources() = default;

    void Dx12Resources::InitAllocator(ID3D12Device* _device, IDXGIAdapter* _adapter)
    {
        D3D12MA::ALLOCATOR_DESC allocatorDesc {
            .pDevice = _device,
            .pAdapter = _adapter,
        };

        Dx12Assert(D3D12MA::CreateAllocator(&allocatorDesc, &m_memoryAllocator));
    }

    void Dx12Resources::InitHeaps(ID3D12Device* _device)
    {
        {
            const D3D12_DESCRIPTOR_HEAP_DESC heapDesc{
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                .NumDescriptors = kCbvSrvUavHeapSize,
                .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            };
            Dx12Assert(_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvSrvUavDescriptorStorageHeap)));
#if !defined(KE_FINAL)
            Dx12SetName(m_cbvSrvUavDescriptorStorageHeap.Get(), L"CBV/SRV/UAV Descriptor Storage Heap");
#endif
        }
    }

    BufferHandle Dx12Resources::CreateBuffer(const BufferCreateDesc& _desc)
    {
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(_desc.m_desc.m_size);

        if (BitUtils::EnumHasAny(_desc.m_usage, MemoryUsage::WriteBuffer))
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }
        if (!BitUtils::EnumHasAny(_desc.m_usage, MemoryUsage::ReadBuffer | MemoryUsage::ConstantBuffer))
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }
        if (BitUtils::EnumHasAny(_desc.m_usage, MemoryUsage::AccelerationStruct))
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        D3D12MA::ALLOCATION_DESC allocationDesc {
            .HeapType = Dx12Converters::GetHeapType(_desc.m_usage),
        };

        D3D12MA::Allocation* allocation;
        ID3D12Resource* buffer;
        Dx12Assert(m_memoryAllocator->CreateResource(
            &allocationDesc,
            &resourceDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            &allocation,
            IID_PPV_ARGS(&buffer)));

#if !defined(KE_FINAL)
        Dx12SetName(buffer, L"%s", _desc.m_desc.m_debugName.c_str());
#endif

        const GenPool::Handle handle = m_buffers.Allocate();
        *m_buffers.Get(handle) = buffer;
        *m_buffers.GetCold(handle) = allocation;

        return { handle };
    }

    BufferHandle Dx12Resources::CreateStagingBuffer(
        const TextureDesc& _desc,
        const eastl::vector<TextureMemoryFootprint>& _footprints)
    {
        const TextureMemoryFootprint lastSubResourceFootPrint = _footprints.back();
        const u64 bufferWidth = lastSubResourceFootPrint.m_offset + lastSubResourceFootPrint.m_lineByteAlignedSize
                                                                        * lastSubResourceFootPrint.m_height
                                                                        * lastSubResourceFootPrint.m_depth;

        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
            bufferWidth,
            D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE,
            0);

        const D3D12MA::ALLOCATION_DESC allocationDesc {
            .HeapType = D3D12_HEAP_TYPE_UPLOAD,
        };

        const GenPool::Handle handle = m_buffers.Allocate();

        Dx12Assert(m_memoryAllocator->CreateResource(
            &allocationDesc,
            &resourceDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            m_buffers.GetCold(handle),
            IID_PPV_ARGS(m_buffers.Get(handle))));


#if !defined(KE_FINAL)
        Dx12SetName(*m_buffers.Get(handle), L"%s staging buffer", _desc.m_debugName.c_str());
#endif

        return { handle };
    }

    bool Dx12Resources::DestroyBuffer(BufferHandle _buffer)
    {
        ID3D12Resource* resource;
        D3D12MA::Allocation* allocation;

        if (m_buffers.Free(_buffer.m_handle, &resource, &allocation))
        {
            SafeRelease(resource);
            allocation->Release();

            return true;
        }

        return false;
    }

    TextureHandle Dx12Resources::CreateTexture(const TextureCreateDesc& _createDesc, ID3D12Device* _device)
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
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, // Uses most efficient layout for hardware.
            .Flags = Dx12Converters::GetTextureResourceFlags(_createDesc.m_memoryUsage),
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

#if !defined(KE_FINAL)
   Dx12SetName(texture, L"%s", _createDesc.m_desc.m_debugName.c_str());
#endif

        return RegisterTexture(texture, allocation);
    }

    TextureHandle Dx12Resources::RegisterTexture(ID3D12Resource* _texture, D3D12MA::Allocation* _allocation)
    {
        const auto handle = m_textures.Allocate();
        *m_textures.Get(handle) = _texture;
        *m_textures.GetCold(handle) = _allocation;
        return { handle };
    }

    bool Dx12Resources::ReleaseTexture(TextureHandle _texture, bool _free)
    {
        ID3D12Resource* texture = nullptr;
        D3D12MA::Allocation* allocation = nullptr;
        if (m_textures.Free(_texture.m_handle, _free ? &texture : nullptr, &allocation))
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

    bool Dx12Resources::DestroyTextureSrv(TextureSrvHandle _textureSrv)
    {
        return m_cbvSrvUav.Free(_textureSrv.m_handle);
    }

    SamplerHandle Dx12Resources::CreateSampler(const SamplerDesc& _samplerDesc, ID3D12Device* _device)
    {
        if (m_samplerStorageHeap == nullptr)
        {
            const D3D12_DESCRIPTOR_HEAP_DESC heapDesc {
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                .NumDescriptors = kSamplerHeapSize,
                .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE // Not shader visible, this is a storage heap
            };
            Dx12Assert(_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_samplerStorageHeap)));
#if !defined(KE_FINAL)
            Dx12SetName(m_samplerStorageHeap.Get(), L"Sampler descriptor storage heap");
#endif
            m_samplerDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        }

        D3D12_SAMPLER_DESC samplerDesc {
            .AddressU = Dx12Converters::ToDx12AddressMode(_samplerDesc.m_addressModeU),
            .AddressV = Dx12Converters::ToDx12AddressMode(_samplerDesc.m_addressModeV),
            .AddressW = Dx12Converters::ToDx12AddressMode(_samplerDesc.m_addressModeW),
            .MipLODBias = _samplerDesc.m_lodBias,
            .MaxAnisotropy = _samplerDesc.m_anisotropy,
            .ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS,
            .BorderColor = {
                _samplerDesc.m_borderColor.x,
                _samplerDesc.m_borderColor.y,
                _samplerDesc.m_borderColor.z,
                _samplerDesc.m_borderColor.w },
            .MinLOD = _samplerDesc.m_lodMin,
            .MaxLOD = _samplerDesc.m_lodMax,
        };

        {
            // Point filtering flag is 0
            int filter = 0;

            if (_samplerDesc.m_minFilter == SamplerDesc::Filter::Linear)
            {
                filter &= D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT; // Min linear flag
            }
            if (_samplerDesc.m_magFilter == SamplerDesc::Filter::Linear)
            {
                filter &= D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; // Mag linear flag
            }
            if (_samplerDesc.m_mipFilter == SamplerDesc::Filter::Linear)
            {
                filter &= D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR; // Mip linear flag
            }

            if (_samplerDesc.m_opType != SamplerDesc::OpType::Blend)
            {
                // Set comparison filter mode
                filter &= D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
            }

            samplerDesc.Filter = static_cast<D3D12_FILTER>(filter);

            // Set comparison operators
            if (_samplerDesc.m_opType == SamplerDesc::OpType::Maximum)
            {
                samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER;
            }
            else if (_samplerDesc.m_opType == SamplerDesc::OpType::Minimum)
            {
                samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
            }
        }

        const GenPool::Handle handle = m_samplers.Allocate();
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle(
            m_samplerStorageHeap->GetCPUDescriptorHandleForHeapStart(),
            handle.m_index,
            m_samplerDescriptorSize);
        _device->CreateSampler(&samplerDesc, cpuDescriptorHandle);

        *m_samplers.Get(handle) = cpuDescriptorHandle;

        return { handle };
    }

    bool Dx12Resources::DestroySampler(SamplerHandle _sampler)
    {
        return m_samplers.Free(_sampler.m_handle);
    }

    RenderTargetViewHandle
    Dx12Resources::CreateRenderTargetView(const RenderTargetViewDesc &_desc, ID3D12Device *_device)
    {
        auto* texture = m_textures.Get(_desc.m_texture.m_handle);
        if (texture == nullptr)
        {
            return { GenPool::kInvalidHandle };
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
            .m_resource = _desc.m_texture,
        };
        *m_renderTargetViews.GetCold(handle) = Dx12Converters::ToDx12Format(_desc.m_format);

        return { handle };
    }

    bool Dx12Resources::FreeRenderTargetView(RenderTargetViewHandle _rtv)
    {
        // Don't have to destroy anything, as the memory slot will be marked as free.
        // Only the heap itself will need to be freed using API.
        return m_renderTargetViews.Free(_rtv.m_handle);
    }

    RenderPassHandle Dx12Resources::CreateRenderPass(const RenderPassDesc &_desc)
    {
        auto handle = m_renderPasses.Allocate();
        auto* desc = m_renderPasses.Get(handle);

        // Manually init pointer location using a copy, as the allocator doesn't initialize its objects.
        new (desc) RenderPassDesc(_desc);
        return { handle };
    }

    bool Dx12Resources::FreeRenderPass(RenderPassHandle _handle)
    {
        // Simply mark slot as available.
        return m_renderPasses.Free(_handle.m_handle);
    }

    TextureSrvHandle Dx12Resources::CreateTextureSrv(const TextureSrvDesc& _srvDesc, ID3D12Device* _device)
    {
        auto* texturePtr = m_textures.Get(_srvDesc.m_texture.m_handle);
        VERIFY_OR_RETURN(texturePtr != nullptr, { GenPool::kInvalidHandle });
        ID3D12Resource* texture = *texturePtr;

        const GenPool::Handle handle = m_cbvSrvUav.Allocate();

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

        switch (_srvDesc.m_viewType)
        {
        case TextureTypes::Single1D:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D = {
                .MostDetailedMip = _srvDesc.m_minMip,
                .MipLevels = static_cast<u32>(_srvDesc.m_maxMip - _srvDesc.m_minMip + 1),
                .ResourceMinLODClamp = 0.f
            };
            break;
        case TextureTypes::Single2D:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D = {
                .MostDetailedMip = _srvDesc.m_minMip,
                .MipLevels = static_cast<u32>(_srvDesc.m_maxMip - _srvDesc.m_minMip + 1),
                .PlaneSlice = _srvDesc.m_arrayStart,
                .ResourceMinLODClamp = 0.f
            };
            break;
        case TextureTypes::Single3D:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            srvDesc.Texture3D = {
                .MostDetailedMip = _srvDesc.m_minMip,
                .MipLevels = static_cast<u32>(_srvDesc.m_maxMip - _srvDesc.m_minMip + 1),
                .ResourceMinLODClamp = 0.f
            };
            break;
        case TextureTypes::Array1D:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray = {
                .MostDetailedMip = _srvDesc.m_minMip,
                .MipLevels = static_cast<u32>(_srvDesc.m_maxMip - _srvDesc.m_minMip + 1),
                .FirstArraySlice = _srvDesc.m_arrayStart,
                .ArraySize = _srvDesc.m_arrayRange,
                .ResourceMinLODClamp = 0.f
            };
            break;
        case TextureTypes::Array2D:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray = {
                .MostDetailedMip = _srvDesc.m_minMip,
                .MipLevels = static_cast<u32>(_srvDesc.m_maxMip - _srvDesc.m_minMip + 1),
                .FirstArraySlice = _srvDesc.m_arrayStart,
                .ArraySize = _srvDesc.m_arrayRange,
                .PlaneSlice = 0,
                .ResourceMinLODClamp = 0.f
            };
            break;
        case TextureTypes::SingleCube:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube = {
                .MostDetailedMip = _srvDesc.m_minMip,
                .MipLevels = static_cast<u32>(_srvDesc.m_maxMip - _srvDesc.m_minMip + 1),
                .ResourceMinLODClamp = 0.f
            };
            break;
        case TextureTypes::ArrayCube:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
            srvDesc.TextureCubeArray = {
                .MostDetailedMip = _srvDesc.m_minMip,
                .MipLevels = static_cast<u32>(_srvDesc.m_maxMip - _srvDesc.m_minMip + 1),
                .First2DArrayFace = _srvDesc.m_arrayStart,
                .NumCubes = _srvDesc.m_arrayRange,
                .ResourceMinLODClamp = 0.f
            };
            break;
        }

        // Create SRV and copy to current shader visible heap
        {
            const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle(
                m_cbvSrvUavDescriptorStorageHeap->GetCPUDescriptorHandleForHeapStart(),
                handle.m_index,
                m_cbvSrvUavDescriptorSize);
            _device->CreateShaderResourceView(texture, &srvDesc, cpuDescriptorHandle);

            *m_cbvSrvUav.Get(handle) = cpuDescriptorHandle;
        }

        return { handle };
    }

    ShaderModuleHandle Dx12Resources::RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize)
    {
        GenPool::Handle handle = m_shaderBytecodes.Allocate();
        *m_shaderBytecodes.Get(handle) = {
            .pShaderBytecode = _bytecodeData,
            .BytecodeLength = _bytecodeSize,
        };
        return { handle };
    }

    PipelineLayoutHandle Dx12Resources::CreatePipelineLayout(
        const PipelineLayoutDesc& _desc,
        Dx12DescriptorSetManager* _setManager,
        ID3D12Device* _device)
    {
        eastl::vector<D3D12_ROOT_PARAMETER> rootParameters;

        eastl::vector<D3D12_DESCRIPTOR_RANGE> ranges {};
        eastl::vector<u32> offsets {};
        for (auto setIndex = 0u; setIndex < _desc.m_descriptorSets.size(); setIndex++)
        {
            auto layout = _desc.m_descriptorSets[setIndex];

            const Dx12DescriptorSetManager::LayoutData* layoutData = _setManager->GetDescriptorSetLayoutData(layout);

            u32 rangesCount = 0;
            const u32 rangesOffset = ranges.size();

            constexpr u32 samplerIndex = static_cast<u32>(Dx12DescriptorSetManager::RangeType::Sampler);

            // Must separate CBV/SRV/UAV descriptor table from Sampler descriptor table, as they live on different
            // descriptor heaps.
            {
                // Set up CBV/SRV/UAV descriptor table
                for (auto i = 0u; i < samplerIndex; i++)
                {
                    if (layoutData->m_totals[i] > 0)
                    {
                        D3D12_DESCRIPTOR_RANGE range{
                            .NumDescriptors = layoutData->m_totals[i],
                            .BaseShaderRegister = 0,
                            .RegisterSpace = setIndex,
                            .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
                        };

                        switch (static_cast<Dx12DescriptorSetManager::RangeType>(i))
                        {
                        case Dx12DescriptorSetManager::RangeType::CBV:
                            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                            break;
                        case Dx12DescriptorSetManager::RangeType::SRV:
                            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                            break;
                        case Dx12DescriptorSetManager::RangeType::UAV:
                            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                            break;
                        default:
                            KE_ERROR("Erroneous value %d", i);
                            continue;
                        }

                        ranges.push_back(range);
                        rangesCount++;
                    }
                }

                if (rangesCount > 0)
                {
                    rootParameters.push_back(D3D12_ROOT_PARAMETER {
                        .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                        .DescriptorTable = {
                            .NumDescriptorRanges = rangesCount,
                        },
                        .ShaderVisibility = Dx12Converters::ToDx12ShaderVisibility(
                            layoutData->m_visibilities[static_cast<u32>(Dx12DescriptorSetManager::RangeType::CBV)] |
                            layoutData->m_visibilities[static_cast<u32>(Dx12DescriptorSetManager::RangeType::SRV)] |
                            layoutData->m_visibilities[static_cast<u32>(Dx12DescriptorSetManager::RangeType::UAV)]),
                    });
                    offsets.push_back(rangesOffset);
                }

                // Set up sampler descriptor table
                if (layoutData->m_totals[samplerIndex] > 0)
                {
                    D3D12_DESCRIPTOR_RANGE range{
                        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
                        .NumDescriptors = layoutData->m_totals[samplerIndex],
                        .BaseShaderRegister = 0,
                        .RegisterSpace = setIndex,
                        .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
                    };

                    rootParameters.push_back(D3D12_ROOT_PARAMETER {
                        .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                        .DescriptorTable = {
                            .NumDescriptorRanges = 1,
                        },
                        .ShaderVisibility = Dx12Converters::ToDx12ShaderVisibility(
                            layoutData->m_visibilities[static_cast<u32>(Dx12DescriptorSetManager::RangeType::Sampler)]),
                    });
                    offsets.push_back(ranges.size());

                    ranges.push_back(range);
                }
            }
        }

        // Set ranges pointers appropriately now that the ranges vector won't grow any more
        KE_ASSERT(rootParameters.size() == offsets.size());
        for (auto i = 0u; i < rootParameters.size(); i++)
        {
            rootParameters[i].DescriptorTable.pDescriptorRanges = ranges.data() + offsets[i];
        }

        for (const auto& pushConstant: _desc.m_pushConstants)
        {
            rootParameters.push_back(D3D12_ROOT_PARAMETER {
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
                .Constants = D3D12_ROOT_CONSTANTS {
                    .ShaderRegister = 0,
                    .RegisterSpace = static_cast<u32>(rootParameters.size()),
                    .Num32BitValues = pushConstant.m_sizeInBytes / 4u,
                },
                .ShaderVisibility = Dx12Converters::ToDx12ShaderVisibility(pushConstant.m_visibility),
            });
        }

        D3D12_ROOT_SIGNATURE_DESC rootDesc {
            .NumParameters = static_cast<u32>(rootParameters.size()),
            .pParameters = rootParameters.data(),
            .Flags = _desc.m_useVertexLayout
                ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
                : D3D12_ROOT_SIGNATURE_FLAG_NONE,
        };

        ID3DBlob* serializedRootBlob;
        ID3DBlob* errorBlob;
        const auto hr = D3D12SerializeRootSignature(
            &rootDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &serializedRootBlob,
            &errorBlob);
        if (!SUCCEEDED(hr))
        {
            KE_ERROR((const char*)errorBlob->GetBufferPointer());
        }
        const GenPool::Handle handle = m_rootSignatures.Allocate();
        Dx12Assert(_device->CreateRootSignature(
            0,
            serializedRootBlob->GetBufferPointer(),
            serializedRootBlob->GetBufferSize(),
            IID_PPV_ARGS(m_rootSignatures.Get(handle))));

        return { handle };
    }

    GraphicsPipelineHandle Dx12Resources::CreateGraphicsPipeline(
        const GraphicsPipelineDesc& _desc,
        ID3D12Device* _device)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc {};

        VERIFY_OR_RETURN(_desc.m_renderPass != GenPool::kInvalidHandle, { GenPool::kInvalidHandle });
        const RenderPassDesc* renderPassDesc = m_renderPasses.Get(_desc.m_renderPass.m_handle);
        VERIFY_OR_RETURN(renderPassDesc != nullptr, { GenPool::kInvalidHandle });

        // Set root signature
        {
            VERIFY_OR_RETURN(_desc.m_pipelineLayout != GenPool::kInvalidHandle, { GenPool::kInvalidHandle });
            ID3D12RootSignature** pSignature = m_rootSignatures.Get(_desc.m_pipelineLayout.m_handle);
            VERIFY_OR_RETURN(pSignature != nullptr, { GenPool::kInvalidHandle });

            desc.pRootSignature = *pSignature;
        }

        // Set shader stages
        for (const auto& stage: _desc.m_stages)
        {
            VERIFY_OR_RETURN(stage.m_shaderModule != GenPool::kInvalidHandle, { GenPool::kInvalidHandle });

            D3D12_SHADER_BYTECODE* pByteCode = m_shaderBytecodes.Get(stage.m_shaderModule.m_handle);
            VERIFY_OR_RETURN(pByteCode != nullptr, { GenPool::kInvalidHandle });
            VERIFY_OR_RETURN(pByteCode->pShaderBytecode != nullptr, { GenPool::kInvalidHandle });

            switch (stage.m_stage)
            {
            case GraphicsShaderStage::Stage::Vertex:
                KE_ASSERT_MSG(desc.VS.pShaderBytecode == nullptr, "Defined vertex shader stage twice");
                desc.VS = *pByteCode;
                break;
            case GraphicsShaderStage::Stage::TesselationControl:
                KE_ASSERT_MSG(desc.HS.pShaderBytecode == nullptr, "Defined tesselation control shader stage twice");
                desc.HS = *pByteCode;
                break;
            case GraphicsShaderStage::Stage::TesselationEvaluation:
                KE_ASSERT_MSG(desc.DS.pShaderBytecode == nullptr, "Defined tesselation evaluation shader stage twice");
                desc.DS = *pByteCode;
                break;
            case GraphicsShaderStage::Stage::Geometry:
                KE_ASSERT_MSG(desc.GS.pShaderBytecode == nullptr, "Defined geometry shader stage twice");
                desc.GS = *pByteCode;
                break;
            case GraphicsShaderStage::Stage::Fragment:
                KE_ASSERT_MSG(desc.PS.pShaderBytecode == nullptr, "Defined fragment shader stage twice");
                desc.PS = *pByteCode;
                break;
            default:
                KE_ERROR("Unsupported shader stage");
                break;
            }
        }

        // Blend state
        {
            const auto& colorBlending = _desc.m_colorBlending;

            desc.BlendState = {
                .AlphaToCoverageEnable = false,
            };

            const D3D12_LOGIC_OP logicOp = Dx12Converters::ToDx12LogicOp(colorBlending.m_logicOp);

            for (auto i = 0u; i < colorBlending.m_attachments.size(); i++)
            {
                const auto& attachmentDesc = colorBlending.m_attachments[i];
                auto& renderTarget = desc.BlendState.RenderTarget[i];

                renderTarget.BlendEnable = attachmentDesc.m_blendEnable;
                renderTarget.LogicOpEnable = colorBlending.m_logicOp != ColorBlendingDesc::LogicOp::None;

                renderTarget.SrcBlend = Dx12Converters::ToDx12Blend(attachmentDesc.m_srcColor);
                renderTarget.DestBlend = Dx12Converters::ToDx12Blend(attachmentDesc.m_dstColor);
                renderTarget.BlendOp = Dx12Converters::ToDx12BlendOp(attachmentDesc.m_colorOp);
                renderTarget.SrcBlendAlpha = Dx12Converters::ToDx12Blend(attachmentDesc.m_srcColor);
                renderTarget.DestBlendAlpha = Dx12Converters::ToDx12Blend(attachmentDesc.m_dstColor);
                renderTarget.BlendOpAlpha = Dx12Converters::ToDx12BlendOp(attachmentDesc.m_alphaOp);

                renderTarget.LogicOp = logicOp;

                renderTarget.RenderTargetWriteMask = static_cast<u8>(attachmentDesc.m_writeMask);
            }

            if (_desc.m_colorBlending.m_logicOp != ColorBlendingDesc::LogicOp::None)
            {
                desc.BlendState.IndependentBlendEnable = false;
            }
        }

        // Sample mask
        {
            desc.SampleMask = 0xffffffff; // TODO: Multisampling support
        }

        // Rasterizer stater
        {
            const auto& rasterState = _desc.m_rasterState;

            switch(rasterState.m_fillMode)
            {
            case RasterStateDesc::FillMode::Wireframe:
                desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
                break;
            case RasterStateDesc::FillMode::Solid:
                desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
                break;
            }

            switch (rasterState.m_cullMode)
            {
            case RasterStateDesc::CullMode::None:
                desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
                break;
            case RasterStateDesc::CullMode::Front:
                desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
                break;
            case RasterStateDesc::CullMode::Back:
                desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
                break;
            }

            desc.RasterizerState.FrontCounterClockwise = rasterState.m_front == RasterStateDesc::Front::CounterClockwise;

            if (rasterState.m_depthBias)
            {
                desc.RasterizerState.DepthBias = *reinterpret_cast<const s32*>(&rasterState.m_depthBiasConstantFactor);
                desc.RasterizerState.DepthBiasClamp = rasterState.m_depthBiasClampValue;
                desc.RasterizerState.SlopeScaledDepthBias = rasterState.m_depthBiasSlopFactor;
            }
            else
            {
                desc.RasterizerState.DepthBias = 0;
                desc.RasterizerState.DepthBiasClamp = 0;
                desc.RasterizerState.SlopeScaledDepthBias = 0;
            }

            desc.RasterizerState.DepthClipEnable = rasterState.m_depthClip;

            // TODO: multisampling support
            desc.RasterizerState.MultisampleEnable = false;
            desc.RasterizerState.AntialiasedLineEnable = false;
            desc.RasterizerState.ForcedSampleCount = 0;

            // TODO: Conservative rasterizing support.
            desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        // Depth stencil desc
        if (renderPassDesc->m_depthStencilAttachment.has_value())
        {
            desc.DepthStencilState.DepthEnable = _desc.m_depthStencil.m_depthTest;
            desc.DepthStencilState.DepthWriteMask = _desc.m_depthStencil.m_depthWrite
                ? D3D12_DEPTH_WRITE_MASK_ALL
                : D3D12_DEPTH_WRITE_MASK_ZERO;
            desc.DepthStencilState.DepthFunc = Dx12Converters::ToDx12CompareFunc(_desc.m_depthStencil.m_depthCompare);

            desc.DepthStencilState.StencilEnable = _desc.m_depthStencil.m_stencilTest;
            desc.DepthStencilState.StencilReadMask = _desc.m_depthStencil.m_stencilReadMask;
            desc.DepthStencilState.StencilWriteMask = _desc.m_depthStencil.m_stencilWriteMask;

            desc.DepthStencilState.FrontFace = D3D12_DEPTH_STENCILOP_DESC {
                .StencilFailOp = Dx12Converters::ToDx12StencilOp(_desc.m_depthStencil.m_front.m_failOp),
                .StencilDepthFailOp = Dx12Converters::ToDx12StencilOp(_desc.m_depthStencil.m_front.m_depthFailOp),
                .StencilPassOp = Dx12Converters::ToDx12StencilOp(_desc.m_depthStencil.m_front.m_passOp),
                .StencilFunc = Dx12Converters::ToDx12CompareFunc(_desc.m_depthStencil.m_front.m_compareOp),
            };

            desc.DepthStencilState.BackFace = D3D12_DEPTH_STENCILOP_DESC {
                .StencilFailOp = Dx12Converters::ToDx12StencilOp(_desc.m_depthStencil.m_back.m_failOp),
                .StencilDepthFailOp = Dx12Converters::ToDx12StencilOp(_desc.m_depthStencil.m_back.m_depthFailOp),
                .StencilPassOp = Dx12Converters::ToDx12StencilOp(_desc.m_depthStencil.m_back.m_passOp),
                .StencilFunc = Dx12Converters::ToDx12CompareFunc(_desc.m_depthStencil.m_back.m_compareOp),
            };
        }

        // Input layout
        eastl::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
        if (!_desc.m_vertexInput.m_elements.empty())
        {
            inputElements.reserve(_desc.m_vertexInput.m_elements.size());
            for (const auto& vertexInput: _desc.m_vertexInput.m_elements)
            {
                inputElements.push_back(D3D12_INPUT_ELEMENT_DESC {
                    .SemanticName = Dx12Converters::ToDx12SemanticName(vertexInput.m_semanticName),
                    .SemanticIndex = vertexInput.m_semanticIndex,
                    .Format = Dx12Converters::ToDx12Format(vertexInput.m_format),
                    .InputSlot = vertexInput.m_bindingIndex,
                    .AlignedByteOffset = vertexInput.m_offset,
                    .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    .InstanceDataStepRate = 0,
                });
            }
            desc.InputLayout.NumElements = inputElements.size();
            desc.InputLayout.pInputElementDescs = inputElements.data();
        }

        // Input assembly
        {
            desc.IBStripCutValue = _desc.m_inputAssembly.m_cutStripAtSpecialIndex
                ? _desc.m_inputAssembly.m_indexSize == InputAssemblyDesc::IndexIntSize::U16
                    ? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF
                    : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF
                : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

            switch (_desc.m_inputAssembly.m_topology)
            {
            case InputAssemblyDesc::PrimitiveTopology::PointList:
                desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case InputAssemblyDesc::PrimitiveTopology::LineList:
            case InputAssemblyDesc::PrimitiveTopology::LineStrip:
                desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case InputAssemblyDesc::PrimitiveTopology::TriangleList:
            case InputAssemblyDesc::PrimitiveTopology::TriangleStrip:
                desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            }
        }

        // Render pass
        {
            desc.NumRenderTargets = renderPassDesc->m_colorAttachments.size();

            for (auto i = 0u; i < desc.NumRenderTargets; i++)
            {
                VERIFY_OR_RETURN(renderPassDesc->m_colorAttachments[i].m_rtv != GenPool::kInvalidHandle, { GenPool::kInvalidHandle });
                auto* pRtvFormat = m_renderTargetViews.GetCold(renderPassDesc->m_colorAttachments[i].m_rtv.m_handle);
                VERIFY_OR_RETURN(pRtvFormat != nullptr, { GenPool::kInvalidHandle });

                desc.RTVFormats[i] = *pRtvFormat;
            }

            if (renderPassDesc->m_depthStencilAttachment.has_value())
            {
                VERIFY_OR_RETURN(renderPassDesc->m_depthStencilAttachment.value().m_rtv != GenPool::kInvalidHandle, { GenPool::kInvalidHandle });
                auto* pRtvFormat = m_renderTargetViews.GetCold(renderPassDesc->m_depthStencilAttachment.value().m_rtv.m_handle);
                VERIFY_OR_RETURN(pRtvFormat != nullptr, { GenPool::kInvalidHandle });

                desc.DSVFormat = *pRtvFormat;
            }
        }

        desc.SampleDesc = {
            .Count = 1,
            .Quality = 0,
        };

        desc.NodeMask = 0;

        const GenPool::Handle handle = m_pipelineStateObjects.Allocate();
        Dx12Assert(_device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pipelineStateObjects.Get(handle))));

#if !defined(KE_FINAL)
        Dx12SetName(*m_pipelineStateObjects.Get(handle), L"%s", _desc.m_debugName.c_str());
#endif

        *m_pipelineStateObjects.GetCold(handle) = {
            .m_signature = desc.pRootSignature,
            .m_topology = _desc.m_inputAssembly.m_topology,
        };

        return { handle };
    }
} // KryneEngine