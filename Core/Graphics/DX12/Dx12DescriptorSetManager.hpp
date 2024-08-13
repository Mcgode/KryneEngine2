/**
 * @file
 * @author Max Godefroy
 * @date 11/08/2024.
 */

#pragma once

#include "Dx12Headers.hpp"
#include "Dx12Types.hpp"
#include <Common/Utils/MultiFrameTracking.hpp>
#include <Graphics/Common/Handles.hpp>

namespace KryneEngine
{
    class Dx12Resources;

    struct DescriptorSetDesc;
    struct DescriptorSetWriteInfo;

    class Dx12DescriptorSetManager
    {
    public:
        void Init(ID3D12Device* _device, u8 _frameContextCount, u8 _currentFrame);

        [[nodiscard]] DescriptorSetHandle CreateDescriptorSet(const DescriptorSetDesc& _setDesc, u32* _bindingIndices);

        void UpdateDescriptorSet(
            DescriptorSetHandle _descriptorSet,
            const Dx12Resources& _resources,
            const eastl::span<DescriptorSetWriteInfo>& _writes,
            ID3D12Device* _device,
            u8 _frameIndex);

        void SetGraphicsDescriptorSets(
            CommandList _commandList,
            const eastl::span<DescriptorSetHandle>& _sets,
            const bool* _unchanged,
            u8 _currentFrame);

        void OnBeginGraphicsCommandList(CommandList _commandList, u8 _frameIndex);

        void NextFrame(ID3D12Device* _device, const Dx12Resources& _resources, u8 _frameIndex);

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
            eastl::array<u32, static_cast<u32>(RangeType::COUNT)> m_offsets;
        };
        GenerationalPool<DescriptorSetRanges> m_descriptorSets;

        struct TrackedData
        {
            DescriptorSetHandle m_descriptorSet;
            GenPool::Handle m_object;
            u32 m_packedIndex;
        };
        MultiFrameDataTracker<TrackedData> m_multiFrameUpdateTracker;

        void _ProcessUpdate(
            ID3D12Device* _device,
            const Dx12Resources& _resources,
            const Dx12DescriptorSetManager::TrackedData& _data,
            u8 _currentFrame);
    };
} // namespace KryneEngine