/**
 * @file
 * @author Max Godefroy
 * @date 23/02/2024.
 */

#pragma once

namespace KryneEngine
{
    struct RenderTargetViewDesc;

    class Dx12Resources
    {
    public:
        Dx12Resources();
        ~Dx12Resources();

        [[nodiscard]] GenPool::Handle RegisterTexture(ID3D12Resource* _texture);

        bool ReleaseTexture(GenPool::Handle _handle, bool _free);

        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc, ID3D12Device* _device);

        bool FreeRenderTargetView(GenPool::Handle _handle);

        GenerationalPool<ID3D12Resource*> m_textures;
        GenerationalPool<CD3DX12_CPU_DESCRIPTOR_HANDLE> m_renderTargetViews;

    private:
        static constexpr u16 kRtvHeapSize = 2048;
        ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap = nullptr;
        u32 m_rtvDescriptorSize = 0;
    };
} // KryneEngine