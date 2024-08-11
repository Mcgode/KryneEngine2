/**
 * @file
 * @author Max Godefroy
 * @date 11/08/2024.
 */

#include "Dx12DescriptorSetManager.hpp"
#include "HelperFunctions.hpp"
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    void Dx12DescriptorSetManager::Init(ID3D12Device* _device, u8 _frameContextCount)
    {
        m_cbvSrvUavGpuDescriptorHeaps.Resize(_frameContextCount);
        m_cbvSrvUavGpuDescriptorHeaps.InitAll(nullptr);
        m_samplerGpuDescriptorHeaps.Resize(_frameContextCount);
        m_samplerGpuDescriptorHeaps.InitAll(nullptr);

        for (auto i = 0u; i < _frameContextCount; i++)
        {
            {
                const D3D12_DESCRIPTOR_HEAP_DESC heapDesc {
                    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                    .NumDescriptors = kCbvSrvUavHeapSize,
                    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                };
                Dx12Assert(_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvSrvUavGpuDescriptorHeaps[i])));
#if !defined(KE_FINAL)
                Dx12SetName(m_cbvSrvUavGpuDescriptorHeaps[i].Get(), L"CBV/SRV/UAV descriptor GPU heap [%d]", i);
#endif
                m_cbvSrvUavDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }

            {
                const D3D12_DESCRIPTOR_HEAP_DESC heapDesc {
                    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                    .NumDescriptors = kSamplerHeapSize,
                    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                };
                Dx12Assert(_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_samplerGpuDescriptorHeaps[i])));
#if !defined(KE_FINAL)
                Dx12SetName(m_samplerGpuDescriptorHeaps[i].Get(), L"Sampler descriptor GPU heap [%d]", i);
#endif
                m_samplerDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            }
        }
    }

    DescriptorSetHandle Dx12DescriptorSetManager::CreateDescriptorSet(
        const DescriptorSetDesc& _setDesc,
        u32* _bindingIndices)
    {
        VERIFY_OR_RETURN(_bindingIndices != nullptr, { GenPool::kInvalidHandle });

        const GenPool::Handle handle = m_descriptorSets.Allocate();
        auto [ranges, desc] = m_descriptorSets.GetAll(handle);
        *desc = _setDesc;

        u16 totals[4] = { 0, 0, 0, 0 };
        for (auto i = 0; i < desc->m_bindings.size(); i++)
        {
            DescriptorBindingDesc binding = desc->m_bindings[i];

            RangeType type;
            switch (binding.m_type)
            {
            case DescriptorBindingDesc::Type::ConstantBuffer:
                type = RangeType::CBV;
                break;
            case DescriptorBindingDesc::Type::SampledTexture:
            case DescriptorBindingDesc::Type::StorageReadOnlyTexture:
            case DescriptorBindingDesc::Type::StorageReadOnlyBuffer:
                type = RangeType::SRV;
                break;
            case DescriptorBindingDesc::Type::StorageReadWriteBuffer:
            case DescriptorBindingDesc::Type::StorageReadWriteTexture:
                type = RangeType::UAV;
                break;
            case DescriptorBindingDesc::Type::Sampler:
                type = RangeType::Sampler;
                break;
            }

            u16& total = totals[static_cast<u32>(type)];
            _bindingIndices[i] = (static_cast<u32>(total) << 16) | static_cast<u32>(type); // Pack index data into a single u32
            total += binding.m_count;
        }

        constexpr u32 samplerIndex = static_cast<u32>(RangeType::Sampler);

        for (auto i = 0u; i < samplerIndex; i++)
        {
            u32 total = 0;
            if (totals[i] > 0)
            {
                ranges->m_sizes[i] = totals[i];
                total += totals[i];
            }

            if (total > 0)
            {
                ranges->m_cbvSrvUavOffset = m_cbvSrvUavLinearAllocIndex.fetch_add(total);
            }
        }

        if (totals[samplerIndex] > 0)
        {
            ranges->m_sizes[samplerIndex] = totals[samplerIndex];
            ranges->m_samplerOffset = m_samplerLinearAllocIndex.fetch_add(totals[samplerIndex]);
        }

        return { handle };
    }

    const DescriptorSetDesc* Dx12DescriptorSetManager::GetDescriptorSetDesc(DescriptorSetHandle _set) const
    {
        VERIFY_OR_RETURN(_set != GenPool::kInvalidHandle, nullptr);
        return m_descriptorSets.GetCold(_set.m_handle);
    }
} // namespace KryneEngine