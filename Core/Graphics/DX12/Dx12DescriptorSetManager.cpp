/**
 * @file
 * @author Max Godefroy
 * @date 11/08/2024.
 */

#include "Dx12DescriptorSetManager.hpp"
#include "Dx12Resources.h"
#include "HelperFunctions.hpp"
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    void Dx12DescriptorSetManager::Init(ID3D12Device* _device, u8 _frameContextCount, u8 _currentFrame)
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

        m_multiFrameUpdateTracker.Init(_frameContextCount, _currentFrame);
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

        u32 total = 0;
        for (auto i = 0u; i < samplerIndex; i++)
        {
            if (totals[i] > 0)
            {
                ranges->m_sizes[i] = totals[i];
                total += totals[i];
            }
        }

        if (total > 0)
        {
            u32 offset = m_cbvSrvUavLinearAllocIndex.fetch_add(total);
            for (auto i = 0u; i < samplerIndex; i++)
            {
                ranges->m_offsets[i] = offset;
                offset += totals[i];
            }
        }

        if (totals[samplerIndex] > 0)
        {
            ranges->m_sizes[samplerIndex] = totals[samplerIndex];
            ranges->m_offsets[samplerIndex] = m_samplerLinearAllocIndex.fetch_add(totals[samplerIndex]);
        }

        return { handle };
    }

    const DescriptorSetDesc* Dx12DescriptorSetManager::GetDescriptorSetDesc(DescriptorSetHandle _set) const
    {
        VERIFY_OR_RETURN(_set != GenPool::kInvalidHandle, nullptr);
        return m_descriptorSets.GetCold(_set.m_handle);
    }

    void Dx12DescriptorSetManager::UpdateDescriptorSet(
        DescriptorSetHandle _descriptorSet,
        const Dx12Resources& _resources,
        const eastl::span<DescriptorSetWriteInfo>& _writes,
        ID3D12Device* _device)
    {
        for (const auto& writeDesc: _writes)
        {
            for (auto i = 0u; i < writeDesc.m_handles.size(); i++)
            {
                TrackedData data {
                    .m_descriptorSet = _descriptorSet,
                    .m_object = writeDesc.m_handles[i],
                    .m_packedIndex = writeDesc.m_index + (static_cast<u32>(writeDesc.m_arrayOffset + i) << 16),
                };
                _ProcessUpdate(_device, _resources, data, 0);
                m_multiFrameUpdateTracker.TrackForOtherFrames(data);
            }
        }
    }

    void Dx12DescriptorSetManager::SetGraphicsDescriptorSets(
        CommandList _commandList,
        const eastl::span<DescriptorSetHandle>& _sets,
        const bool* _unchanged,
        u8 _currentFrame)
    {
        constexpr u32 samplerIndex = static_cast<u32>(RangeType::Sampler);

        u32 tableIndex = 0;
        for (auto setIndex = 0u; setIndex < _sets.size(); setIndex++)
        {
            DescriptorSetHandle set = _sets[setIndex];
            DescriptorSetRanges* pRanges = m_descriptorSets.Get(set.m_handle);
            VERIFY_OR_RETURN_VOID(pRanges != nullptr);

            bool unchanged = _unchanged != nullptr && _unchanged[setIndex];

            u32 cbvSrvUavTotal = 0;
            for (auto i = 0u; i < samplerIndex; i++) { cbvSrvUavTotal += pRanges->m_sizes[i]; }

            if (cbvSrvUavTotal > 0)
            {
                if (!unchanged)
                {
                    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(
                        m_cbvSrvUavGpuDescriptorHeaps[_currentFrame]->GetGPUDescriptorHandleForHeapStart(),
                        pRanges->m_offsets[0],
                        m_cbvSrvUavDescriptorSize);
                    _commandList->SetGraphicsRootDescriptorTable(tableIndex, handle);
                }
                tableIndex++;
            }

            if (pRanges->m_sizes[samplerIndex] > 0)
            {
                if (!unchanged)
                {
                    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(
                        m_samplerGpuDescriptorHeaps[_currentFrame]->GetGPUDescriptorHandleForHeapStart(),
                        pRanges->m_offsets[samplerIndex],
                        m_samplerDescriptorSize);
                    _commandList->SetGraphicsRootDescriptorTable(tableIndex, handle);
                }
                tableIndex++;
            }
        }
    }

    void Dx12DescriptorSetManager::NextFrame(ID3D12Device* _device, const Dx12Resources& _resources, u8 _frameIndex)
    {
        m_multiFrameUpdateTracker.AdvanceToNextFrame();

        for (const auto& data : m_multiFrameUpdateTracker.GetData())
        {
            _ProcessUpdate(_device, _resources, data, _frameIndex);
        }

        m_multiFrameUpdateTracker.ClearData();
    }

    void Dx12DescriptorSetManager::_ProcessUpdate(
        ID3D12Device* _device,
        const Dx12Resources& _resources,
        const Dx12DescriptorSetManager::TrackedData& _data,
        u8 _currentFrame)
    {
        const auto type = static_cast<RangeType>(_data.m_packedIndex & BitUtils::BitMask<u32>(16));
        const bool isSampler = type == RangeType::Sampler;

        auto* dstHeap = (isSampler ? m_samplerGpuDescriptorHeaps : m_cbvSrvUavGpuDescriptorHeaps)[_currentFrame].Get();

        auto* pSrcCpuHandle = _resources.m_cbvSrvUav.Get(_data.m_object);
        if (pSrcCpuHandle == nullptr)
        {
            return;
        }

        const u32 relativeIndex = _data.m_packedIndex >> 16;
        DescriptorSetRanges* pRanges = m_descriptorSets.Get(_data.m_descriptorSet.m_handle);
        if (pRanges == nullptr)
        {
            return;
        }

        u32 index = relativeIndex + pRanges->m_offsets[static_cast<u32>(type)];

        CD3DX12_CPU_DESCRIPTOR_HANDLE dstCpuHandle(
            dstHeap->GetCPUDescriptorHandleForHeapStart(),
            index,
            isSampler ? m_samplerDescriptorSize : m_cbvSrvUavDescriptorSize);

        _device->CopyDescriptorsSimple(
            1,
            dstCpuHandle,
            *pSrcCpuHandle,
            isSampler ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
} // namespace KryneEngine