/**
 * @file
 * @author Max Godefroy
 * @date 23/02/2024.
 */

#pragma once

#include <Common/Arrays.hpp>
#include <Common/Utils/MultiFrameTracking.hpp>
#include <Graphics/Common/RenderPass.hpp>

namespace D3D12MA
{
    class Allocator;
    class Allocation;
}

namespace KryneEngine
{
    struct TextureCreateDesc;
    struct TextureSrvDesc;
    struct RenderTargetViewDesc;

    class Dx12FrameContext;

    class Dx12Resources
    {
    public:
        Dx12Resources();
        ~Dx12Resources();

        void InitAllocator(ID3D12Device* _device, IDXGIAdapter* _adapter);
        void InitHeaps(ID3D12Device* _device, u32 _frameContextCount, u32 _frameIndex);

        [[nodiscard]] GenPool::Handle CreateTexture(const TextureCreateDesc& _createDesc, ID3D12Device* _device);

        [[nodiscard]] GenPool::Handle RegisterTexture(ID3D12Resource* _texture, D3D12MA::Allocation* _allocation = nullptr);

        bool ReleaseTexture(GenPool::Handle _handle, bool _free);

        [[nodiscard]] GenPool::Handle CreateTextureSrv(
            const TextureSrvDesc& _srvDesc,
            ID3D12Device* _device,
            u32 _frameIndex);

        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc, ID3D12Device* _device);
        bool FreeRenderTargetView(GenPool::Handle _handle);

        [[nodiscard]] GenPool::Handle CreateRenderPass(const RenderPassDesc& _desc);
        bool FreeRenderPass(GenPool::Handle _handle);

        void NextFrame(ID3D12Device* _device, u8 _frameIndex);

        struct RtvHotData
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
            GenPool::Handle m_resource;
        };

        GenerationalPool<ID3D12Resource*, D3D12MA::Allocation*> m_textures;
        GenerationalPool<CD3DX12_CPU_DESCRIPTOR_HANDLE> m_cbvSrvUav;
        GenerationalPool<RtvHotData> m_renderTargetViews;
        GenerationalPool<RenderPassDesc> m_renderPasses;

    private:
        static constexpr u16 kRtvHeapSize = 2048;
        ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap = nullptr;
        u32 m_rtvDescriptorSize = 0;

        static_assert(sizeof(GenPool::IndexType) == 2, "GenPool index type changed, please update size appropriately.");
        static constexpr u64 kCbvSrvUavHeapSize = 1u << 16;
        ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavDescriptorStorageHeap;
        DynamicArray<ComPtr<ID3D12DescriptorHeap>> m_cbvSrvUavDescriptorHeaps;
        MultiFrameDataTracker<GenPool::Handle> m_cbvSrvUavDescriptorCopyTracker;
        u32 m_cbvSrvUavDescriptorSize = 0;

        D3D12MA::Allocator* m_memoryAllocator;

    };
} // KryneEngine