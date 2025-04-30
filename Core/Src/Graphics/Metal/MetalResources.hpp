/**
 * @file
 * @author Max Godefroy
 * @date 30/10/2024.
 */

#pragma once

#include <EASTL/fixed_vector.h>

#include "Graphics/Metal/Helpers/RenderState.hpp"
#include "Graphics/Metal/MetalHeaders.hpp"
#include "KryneEngine/Core/Graphics/Enums.hpp"
#include "KryneEngine/Core/Graphics/Handles.hpp"
#include "KryneEngine/Core/Graphics/RenderPass.hpp"
#include "KryneEngine/Core/Memory/GenerationalPool.hpp"

namespace KryneEngine
{
    struct BufferViewDesc;
    struct BufferCreateDesc;
    struct RenderTargetViewDesc;
    struct RenderPassDesc;
    struct SamplerDesc;
    struct TextureCreateDesc;
    struct TextureSrvDesc;

    class MetalArgumentBufferManager;

    class MetalResources
    {
        friend class MetalGraphicsContext;
        friend class MetalArgumentBufferManager;

    public:
        explicit MetalResources(AllocatorInstance _allocator);
        ~MetalResources();

    private:
        [[nodiscard]] AllocatorInstance GetAllocator() const;

    public:
        BufferHandle CreateBuffer(MTL::Device& _device, const BufferCreateDesc& _desc);
        bool DestroyBuffer(BufferHandle _buffer);

    private:
        struct BufferHotData
        {
            NsPtr<MTL::Buffer> m_buffer;
        };

        struct BufferColdData
        {
            MTL::ResourceOptions m_options;
        };

        GenerationalPool<BufferHotData, BufferColdData> m_buffers;

    public:
        TextureHandle CreateTexture(MTL::Device& _device, const TextureCreateDesc& _desc);
        TextureHandle RegisterTexture(MTL::Texture* _texture);
        bool UnregisterTexture(TextureHandle _handle);
        void UpdateSystemTexture(TextureHandle _handle, MTL::Texture* _texture);

    private:
        struct TextureHotData
        {
            NsPtr<MTL::Texture> m_texture;
            bool m_isSystemTexture;
        };

        GenerationalPool<TextureHotData> m_textures;

    public:
        [[nodiscard]] SamplerHandle CreateSampler(MTL::Device& _device, const SamplerDesc& _desc);
        bool DestroySampler(SamplerHandle _sampler);

    private:
        struct SamplerHotData
        {
            NsPtr<MTL::SamplerState> m_sampler;
        };

        GenerationalPool<SamplerHotData> m_samplers;

    public:
        TextureSrvHandle RegisterTextureSrv(const TextureSrvDesc& _desc);
        bool UnregisterTextureSrv(TextureSrvHandle _textureSrv);

    private:
        struct TextureSrvHotData
        {
            NsPtr<MTL::Texture> m_texture;
        };

        GenerationalPool<TextureSrvHotData> m_textureSrvs;

    public:
        [[nodiscard]] BufferViewHandle RegisterBufferView(const BufferViewDesc& _viewDesc);
        bool UnregisterBufferView(BufferViewHandle _handle);

    private:
        struct BufferViewHotData
        {
            NsPtr<MTL::Buffer> m_buffer;
            size_t m_offset;
        };

        GenerationalPool<BufferViewHotData> m_bufferViews;

    public:
        RenderTargetViewHandle RegisterRtv(const RenderTargetViewDesc& _desc);
        bool UnregisterRtv(RenderTargetViewHandle _handle);

        RenderTargetViewHandle RegisterRtv(const RenderTargetViewDesc& _desc, MTL::Texture* _texture);
        void UpdateSystemTexture(RenderTargetViewHandle _handle, MTL::Texture* _newTexture);

    private:
        struct RtvHotData
        {
            NsPtr<MTL::Texture> m_texture;
            bool m_isSystemTexture;
        };

        struct RtvColdData
        {
            TextureFormat m_pixelFormat;
            u16 m_slice;
            u16 m_depthSlice;
            u8 m_mipLevel;
            TexturePlane m_plane;
        };

        GenerationalPool<RtvHotData, RtvColdData> m_renderTargetViews;

    public:
        RenderPassHandle CreateRenderPassDescriptor(const RenderPassDesc& _desc);
        bool DestroyRenderPassDescriptor(RenderPassHandle _handle);

    private:
        struct RenderPassHotData
        {
            NsPtr<MTL::RenderPassDescriptor> m_descriptor;

            struct SystemRtv
            {
                RenderTargetViewHandle m_handle;
                u8 m_index;
            };
            eastl::fixed_vector<SystemRtv, 1> m_systemRtvs;

#if !defined(KE_FINAL)
            eastl::string m_debugName;
#endif
        };

        struct RenderPassColdData
        {
            eastl::fixed_vector<TextureFormat, RenderPassDesc::kMaxSupportedColorAttachments, false> m_colorFormats;
            TextureFormat m_depthStencilFormat;
        };

        GenerationalPool<RenderPassHotData, RenderPassColdData> m_renderPasses;

    public:
        ShaderModuleHandle LoadLibrary(MTL::Device& _device, void* _bytecode, size_t _size);
        bool FreeLibrary(ShaderModuleHandle _library);

    private:
        struct ShaderModuleHotData
        {
            NsPtr<MTL::Library> m_library;
        };

        GenerationalPool<ShaderModuleHotData> m_libraries;

    public:
        [[nodiscard]] GraphicsPipelineHandle CreateGraphicsPso(
            MTL::Device& _device,
            MetalArgumentBufferManager& _argBufferManager,
            const GraphicsPipelineDesc& _desc);
        bool DestroyGraphicsPso(GraphicsPipelineHandle _pso);

    private:
        struct GraphicsPsoHotData
        {
            NsPtr<MTL::RenderPipelineState> m_pso;
            NsPtr<MTL::DepthStencilState> m_depthStencilState;
            RenderDynamicState m_staticState;
            InputAssemblyDesc::PrimitiveTopology m_topology;
            bool m_dynamicBlendFactor;
            bool m_dynamicStencilRef;
            u8 m_vertexBufferFirstIndex;
        };

        GenerationalPool<GraphicsPsoHotData> m_graphicsPso;

    public:
        [[nodiscard]] ComputePipelineHandle CreateComputePso(
            MTL::Device& _device,
            MetalArgumentBufferManager& _argBufferManager,
            const ComputePipelineDesc& _desc);
        bool DestroyComputePso(ComputePipelineHandle _pso);

    private:
        struct ComputePsoHotData
        {
            NsPtr<MTL::ComputePipelineState> m_pso;
        };

        GenerationalPool<ComputePsoHotData> m_computePso;
    };
} // namespace KryneEngine
