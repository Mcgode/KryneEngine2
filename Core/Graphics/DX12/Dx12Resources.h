/**
 * @file
 * @author Max Godefroy
 * @date 23/02/2024.
 */

#pragma once

#include <D3D12MemAlloc.h>
#include <Graphics/Common/RenderPass.hpp>

namespace D3D12MA
{
    class Allocator;
    class Allocation;
}

namespace KryneEngine
{
    struct TextureCreateDesc;
    struct RenderTargetViewDesc;

    class Dx12Resources
    {
    public:
        Dx12Resources();
        ~Dx12Resources();

        void Init(ID3D12Device* _device, IDXGIAdapter* _adapter);

        [[nodiscard]] GenPool::Handle CreateTexture(const TextureCreateDesc& _createDesc, ID3D12Device* _device);

        [[nodiscard]] GenPool::Handle RegisterTexture(ID3D12Resource* _texture, D3D12MA::Allocation* _allocation = nullptr);

        bool ReleaseTexture(GenPool::Handle _handle, bool _free);

        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc, ID3D12Device* _device);
        bool FreeRenderTargetView(GenPool::Handle _handle);

        [[nodiscard]] GenPool::Handle CreateRenderPass(const RenderPassDesc& _desc);
        bool FreeRenderPass(GenPool::Handle _handle);

        struct RtvHotData
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
            GenPool::Handle m_resource;
        };

        GenerationalPool<ID3D12Resource*, D3D12MA::Allocation*> m_textures;
        GenerationalPool<RtvHotData> m_renderTargetViews;
        GenerationalPool<RenderPassDesc> m_renderPasses;

    private:
        static constexpr u16 kRtvHeapSize = 2048;
        ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap = nullptr;
        u32 m_rtvDescriptorSize = 0;

        D3D12MA::Allocator* m_memoryAllocator;
    };
} // KryneEngine