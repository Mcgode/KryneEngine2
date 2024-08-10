/**
 * @file
 * @author Max Godefroy
 * @date 23/02/2024.
 */

#pragma once

#include <Common/Arrays.hpp>
#include <Common/Utils/MultiFrameTracking.hpp>
#include <Graphics/Common/Handles.hpp>
#include <Graphics/Common/RenderPass.hpp>
#include <Graphics/Common/ShaderPipeline.hpp>
#include <Graphics/Common/Texture.hpp>

namespace D3D12MA
{
    class Allocator;
    class Allocation;
}

namespace KryneEngine
{
    struct BufferCreateDesc;
    struct TextureSrvDesc;
    struct RenderTargetViewDesc;

    class Dx12FrameContext;

    class Dx12Resources
    {
        friend class Dx12GraphicsContext;

    public:
        Dx12Resources();
        ~Dx12Resources();

        void InitAllocator(ID3D12Device* _device, IDXGIAdapter* _adapter);
        void InitHeaps(ID3D12Device* _device, u32 _frameContextCount, u32 _frameIndex);

        [[nodiscard]] BufferHandle CreateBuffer(const BufferCreateDesc& _desc);
        [[nodiscard]] BufferHandle CreateStagingBuffer(
            const TextureDesc& _desc,
            const eastl::vector<TextureMemoryFootprint>& _footprints);
        bool DestroyBuffer(BufferHandle _buffer);

        [[nodiscard]] TextureHandle CreateTexture(const TextureCreateDesc& _createDesc, ID3D12Device* _device);
        [[nodiscard]] TextureHandle RegisterTexture(ID3D12Resource* _texture, D3D12MA::Allocation* _allocation = nullptr);
        bool ReleaseTexture(TextureHandle _texture, bool _free);

        [[nodiscard]] TextureSrvHandle CreateTextureSrv(
            const TextureSrvDesc& _srvDesc,
            ID3D12Device* _device,
            u32 _frameIndex);
        bool DestroyTextureSrv(TextureSrvHandle _textureSrv);

        [[nodiscard]] RenderTargetViewHandle CreateRenderTargetView(const RenderTargetViewDesc& _desc, ID3D12Device* _device);
        bool FreeRenderTargetView(RenderTargetViewHandle _rtv);

        [[nodiscard]] RenderPassHandle CreateRenderPass(const RenderPassDesc& _desc);
        bool FreeRenderPass(RenderPassHandle _handle);

        [[nodiscard]] ShaderModuleHandle RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize);
        [[nodiscard]] PipelineLayoutHandle CreatePipelineLayout(const PipelineLayoutDesc& _desc, ID3D12Device* _device);
        [[nodiscard]] GraphicsPipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc, ID3D12Device* _device);

        void NextFrame(ID3D12Device* _device, u8 _frameIndex);

        struct RtvHotData
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuHandle {};
            TextureHandle m_resource {};
        };

        struct PsoColdData
        {
            ID3D12RootSignature* m_signature;
            InputAssemblyDesc::PrimitiveTopology m_topology;
        };

        GenerationalPool<ID3D12Resource*, D3D12MA::Allocation*> m_buffers;
        GenerationalPool<ID3D12Resource*, D3D12MA::Allocation*> m_textures;
        GenerationalPool<CD3DX12_CPU_DESCRIPTOR_HANDLE> m_cbvSrvUav;
        GenerationalPool<RtvHotData, DXGI_FORMAT> m_renderTargetViews;
        GenerationalPool<RenderPassDesc> m_renderPasses;
        GenerationalPool<ID3D12RootSignature*> m_rootSignatures;
        GenerationalPool<D3D12_SHADER_BYTECODE> m_shaderBytecodes;
        GenerationalPool<ID3D12PipelineState*, PsoColdData> m_pipelineStateObjects;

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

        D3D12MA::Allocator* m_memoryAllocator = nullptr;
    };
} // KryneEngine