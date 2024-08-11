/**
 * @file
 * @author Max Godefroy
 * @date 11/08/2024.
 */

#pragma once

#include "Dx12Headers.hpp"
#include <Graphics/Common/Handles.hpp>

namespace KryneEngine
{
    struct DescriptorSetDesc;

    class Dx12DescriptorSetManager
    {
    public:
        void Init(ID3D12Device* _device, u8 _frameContextCount);

        [[nodiscard]] DescriptorSetHandle CreateDescriptorSet(const DescriptorSetDesc& _setDesc, u32* _bindingIndices);

        [[nodiscard]] const DescriptorSetDesc* GetDescriptorSetDesc(DescriptorSetHandle _set) const;

        enum class RangeType: u32
        {
            CBV = 0,
            SRV,
            UAV,
            Sampler,
            COUNT
        };

    private:
        static constexpr u32 kCbvSrvUavHeapSize = 1024;
        DynamicArray<ComPtr<ID3D12DescriptorHeap>> m_cbvSrvUavGpuDescriptorHeaps;
        u32 m_cbvSrvUavDescriptorSize = 0;
        std::atomic<u32> m_cbvSrvUavLinearAllocIndex { 0 };

        static constexpr u32 kSamplerHeapSize = 64;
        DynamicArray<ComPtr<ID3D12DescriptorHeap>> m_samplerGpuDescriptorHeaps;
        u32 m_samplerDescriptorSize = 0;
        std::atomic<u32> m_samplerLinearAllocIndex { 0 };

        struct DescriptorSetRanges
        {
            eastl::array<u16, static_cast<u32>(RangeType::COUNT)> m_sizes;
            u32 m_cbvSrvUavOffset;
            u32 m_samplerOffset;
        };
        GenerationalPool<DescriptorSetRanges, DescriptorSetDesc> m_descriptorSets;
    };
} // namespace KryneEngine