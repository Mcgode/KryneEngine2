/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "KryneEngine/Core/Graphics/Common/GraphicsContext.hpp"

#include "KryneEngine/Core/Graphics/Common/EnumHelpers.hpp"
#include "KryneEngine/Core/Window/Window.hpp"

#if defined(KE_GRAPHICS_API_VK)
#   include "KryneEngine/Core/Graphics/Vulkan/VkGraphicsContext.hpp"
#elif defined(KE_GRAPHICS_API_DX12)
#   include "KryneEngine/Core/Graphics/DirectX12/Dx12GraphicsContext.hpp"
#elif defined(KE_GRAPHICS_API_MTL)
#   include "KryneEngine/Core/Graphics/Metal/MetalGraphicsContext.hpp"
#else
#   error No valid graphics API
#endif

namespace KryneEngine
{
#if defined(KE_GRAPHICS_API_VK)

    struct GraphicsContextBlob
    {
        GraphicsContext m_interface;
        VkGraphicsContext m_implementation;
    };

#elif defined(KE_GRAPHICS_API_DX12)

    struct GraphicsContextBlob
    {
        GraphicsContext m_interface;
        Dx12GraphicsContext m_implementation;
    };

#elif defined(KE_GRAPHICS_API_MTL)

    struct GraphicsContextBlob
    {
        GraphicsContext m_interface;
        MetalGraphicsContext m_implementation;
    };

#endif

    static_assert(offsetof(GraphicsContextBlob, m_interface) == 0, "Blob must start with interface struct");

    using Implementation = decltype(GraphicsContextBlob::m_implementation);

    Implementation& GetImplementation(GraphicsContext* _this)
    {
        return reinterpret_cast<GraphicsContextBlob*>(_this)->m_implementation;
    }

    const Implementation& GetImplementation(const GraphicsContext* _this)
    {
        return reinterpret_cast<const GraphicsContextBlob*>(_this)->m_implementation;
    }

    GraphicsContext* GraphicsContext::Create(const GraphicsCommon::ApplicationInfo& _appInfo, Window* _window)
    {
        auto* blob = new GraphicsContextBlob {
            .m_interface = {},
            .m_implementation = Implementation(_appInfo, _window, kInitialFrameId),
        };
        blob->m_interface.m_frameId = kInitialFrameId;
        blob->m_interface.m_window = _window;

        return &blob->m_interface;
    }
    void GraphicsContext::Destroy(GraphicsContext* _context)
    {
        delete reinterpret_cast<GraphicsContextBlob*>(_context);
    }

    u8 GraphicsContext::GetFrameContextCount() const
    {
        return GetImplementation(this).GetFrameContextCount();
    }

    bool GraphicsContext::EndFrame()
    {
        GetImplementation(this).EndFrame(m_frameId);
        m_frameId++;
        return m_window->WaitForEvents();
    }

    void GraphicsContext::WaitForLastFrame() const
    {
        GetImplementation(this).WaitForFrame(m_frameId - 1);
    }

    bool GraphicsContext::IsFrameExecuted(u64 _frameId) const
    {
        return GetImplementation(this).IsFrameExecuted(_frameId);
    }

    const GraphicsCommon::ApplicationInfo& GraphicsContext::GetApplicationInfo() const
    {
        return GetImplementation(this).GetApplicationInfo();
    }

    const char* GraphicsContext::GetShaderFileExtension()
    {
#if defined(KE_GRAPHICS_API_VK)
        return "spv";
#elif defined(KE_GRAPHICS_API_DX12)
        return "cso";
#elif defined(KE_GRAPHICS_API_MTL)
        return "metallib";
#else
        static_assert("Not yet implemented");
        return nullptr;
#endif
    }

    BufferHandle GraphicsContext::CreateBuffer(const BufferCreateDesc& _desc)
    {
        return GetImplementation(this).CreateBuffer(_desc);
    }

    bool GraphicsContext::NeedsStagingBuffer(BufferHandle _buffer)
    {
        return GetImplementation(this).NeedsStagingBuffer(_buffer);
    }
    bool GraphicsContext::DestroyBuffer(BufferHandle _bufferHandle)
    {
        return GetImplementation(this).DestroyBuffer(_bufferHandle);
    }

    TextureHandle GraphicsContext::CreateTexture(const TextureCreateDesc& _createDesc)
    {
        if (!KE_VERIFY_MSG(
                (_createDesc.m_memoryUsage & MemoryUsage::USAGE_TYPE_MASK) == MemoryUsage::GpuOnly_UsageType,
                "The engine is designed around having buffers representing textures on the CPU")) [[unlikely]]
        {
            return { GenPool::kInvalidHandle };
        }

        VERIFY_OR_RETURN(
            _createDesc.m_desc.m_dimensions.x != 0
                && _createDesc.m_desc.m_dimensions.y != 0
                && _createDesc.m_desc.m_dimensions.z != 0
                && _createDesc.m_desc.m_arraySize != 0
                && _createDesc.m_desc.m_mipCount != 0,
            { GenPool::kInvalidHandle });

        VERIFY_OR_RETURN(BitUtils::EnumHasAny(_createDesc.m_memoryUsage, ~MemoryUsage::USAGE_TYPE_MASK),
                         { GenPool::kInvalidHandle });

        VERIFY_OR_RETURN(
            !(BitUtils::EnumHasAny(_createDesc.m_memoryUsage, MemoryUsage::DepthStencilTargetImage)
                ^ GraphicsEnumHelpers::IsDepthOrStencilFormat(_createDesc.m_desc.m_format)),
            { GenPool::kInvalidHandle });

        return GetImplementation(this).CreateTexture(_createDesc);
    }

    eastl::vector<TextureMemoryFootprint> GraphicsContext::FetchTextureSubResourcesMemoryFootprints(
        const TextureDesc& _desc)
    {
        return GetImplementation(this).FetchTextureSubResourcesMemoryFootprints(_desc);
    }

    BufferHandle GraphicsContext::CreateStagingBuffer(
        const TextureDesc& _createDesc,
        const eastl::vector<TextureMemoryFootprint>& _footprints)
    {
        return GetImplementation(this).CreateStagingBuffer(_createDesc, _footprints);
    }

    bool GraphicsContext::DestroyTexture(TextureHandle _handle)
    {
        return GetImplementation(this).DestroyTexture(_handle);
    }

    TextureSrvHandle GraphicsContext::CreateTextureSrv(const TextureSrvDesc& _srvDesc)
    {
        return GetImplementation(this).CreateTextureSrv(_srvDesc, m_frameId);
    }

    bool GraphicsContext::DestroyTextureSrv(TextureSrvHandle _handle)
    {
        return GetImplementation(this).DestroyTextureSrv(_handle);
    }

    SamplerHandle GraphicsContext::CreateSampler(const SamplerDesc& _samplerDesc)
    {
        return GetImplementation(this).CreateSampler(_samplerDesc);
    }

    bool GraphicsContext::DestroySampler(SamplerHandle _sampler)
    {
        return GetImplementation(this).DestroySampler(_sampler);
    }

    RenderTargetViewHandle GraphicsContext::CreateRenderTargetView(const RenderTargetViewDesc& _desc)
    {
        return GetImplementation(this).CreateRenderTargetView(_desc);
    }

    bool GraphicsContext::DestroyRenderTargetView(RenderTargetViewHandle _handle)
    {
        return GetImplementation(this).DestroyRenderTargetView(_handle);
    }

    RenderTargetViewHandle GraphicsContext::GetPresentRenderTargetView(u8 _swapChainIndex)
    {
        return GetImplementation(this).GetPresentRenderTargetView(_swapChainIndex);
    }

    TextureHandle GraphicsContext::GetPresentTexture(u8 _swapChainIndex)
    {
        return GetImplementation(this).GetPresentTexture(_swapChainIndex);
    }

    u32 GraphicsContext::GetCurrentPresentImageIndex() const
    {
        return GetImplementation(this).GetCurrentPresentImageIndex();
    }

    RenderPassHandle GraphicsContext::CreateRenderPass(const RenderPassDesc& _desc)
    {
        return GetImplementation(this).CreateRenderPass(_desc);
    }

    bool GraphicsContext::DestroyRenderPass(RenderPassHandle _handle)
    {
        return GetImplementation(this).DestroyRenderPass(_handle);
    }

    CommandListHandle GraphicsContext::BeginGraphicsCommandList()
    {
        return reinterpret_cast<CommandListHandle>(GetImplementation(this).BeginGraphicsCommandList(m_frameId));
    }

    void GraphicsContext::EndGraphicsCommandList()
    {
        GetImplementation(this).EndGraphicsCommandList(m_frameId);
    }

    void GraphicsContext::BeginRenderPass(CommandListHandle _commandList, RenderPassHandle _handle)
    {
        GetImplementation(this).BeginRenderPass(
            reinterpret_cast<CommandList>(_commandList),
            _handle);
    }

    void GraphicsContext::EndRenderPass(CommandListHandle _commandList)
    {
        GetImplementation(this).EndRenderPass(reinterpret_cast<CommandList>(_commandList));
    }

    void GraphicsContext::SetTextureData(
        CommandListHandle _commandList,
        BufferHandle _stagingBuffer,
        TextureHandle _dstTexture,
        const TextureMemoryFootprint& _footprint,
        const SubResourceIndexing& _subResourceIndex,
        void* _data)
    {
        GetImplementation(this).SetTextureData(
            reinterpret_cast<CommandList>(_commandList),
            _stagingBuffer,
            _dstTexture,
            _footprint,
            _subResourceIndex,
            _data);
    }

    void GraphicsContext::MapBuffer(BufferMapping& _mapping)
    {
        GetImplementation(this).MapBuffer(_mapping);
    }

    void GraphicsContext::UnmapBuffer(BufferMapping& _mapping)
    {
        GetImplementation(this).UnmapBuffer(_mapping);
    }

    void GraphicsContext::CopyBuffer(CommandListHandle _commandList, const BufferCopyParameters& _params)
    {
        GetImplementation(this).CopyBuffer(
            reinterpret_cast<CommandList>(_commandList),
            _params);
    }

    void GraphicsContext::PlaceMemoryBarriers(
        CommandListHandle _commandList,
        const eastl::span<GlobalMemoryBarrier>& _globalMemoryBarriers,
        const eastl::span<BufferMemoryBarrier>& _bufferMemoryBarriers,
        const eastl::span<TextureMemoryBarrier>& _textureMemoryBarriers)
    {
        return GetImplementation(this).PlaceMemoryBarriers(
            reinterpret_cast<CommandList>(_commandList),
            _globalMemoryBarriers,
            _bufferMemoryBarriers,
            _textureMemoryBarriers);
    }


    void GraphicsContext::DeclarePassTextureSrvUsage(
        CommandListHandle _commandList,
        const eastl::span<TextureSrvHandle>& _textures)
    {
        GetImplementation(this).DeclarePassTextureSrvUsage(
            reinterpret_cast<CommandList>(_commandList),
            _textures);
    }

    ShaderModuleHandle GraphicsContext::RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize)
    {
        return GetImplementation(this).RegisterShaderModule(_bytecodeData, _bytecodeSize);
    }

    DescriptorSetLayoutHandle GraphicsContext::CreateDescriptorSetLayout(
        const DescriptorSetDesc& _desc,
        u32* _bindingIndices)
    {
        return GetImplementation(this).CreateDescriptorSetLayout(_desc, _bindingIndices);
    }

    DescriptorSetHandle GraphicsContext::CreateDescriptorSet(DescriptorSetLayoutHandle _layout)
    {
        return GetImplementation(this).CreateDescriptorSet(_layout);
    }

    PipelineLayoutHandle GraphicsContext::CreatePipelineLayout(const PipelineLayoutDesc& _desc)
    {
        return GetImplementation(this).CreatePipelineLayout(_desc);
    }

    GraphicsPipelineHandle GraphicsContext::CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc)
    {
        return GetImplementation(this).CreateGraphicsPipeline(_desc);
    }

    bool GraphicsContext::DestroyGraphicsPipeline(GraphicsPipelineHandle _pipeline)
    {
        return GetImplementation(this).DestroyGraphicsPipeline(_pipeline);
    }

    bool GraphicsContext::DestroyPipelineLayout(PipelineLayoutHandle _layout)
    {
        return GetImplementation(this).DestroyPipelineLayout(_layout);
    }

    bool GraphicsContext::DestroyDescriptorSet(DescriptorSetHandle _set)
    {
        return GetImplementation(this).DestroyDescriptorSet(_set);
    }

    bool GraphicsContext::DestroyDescriptorSetLayout(DescriptorSetLayoutHandle _layout)
    {
        return GetImplementation(this).DestroyDescriptorSetLayout(_layout);
    }

    bool GraphicsContext::FreeShaderModule(ShaderModuleHandle _module)
    {
        return GetImplementation(this).FreeShaderModule(_module);
    }

    void GraphicsContext::UpdateDescriptorSet(
        DescriptorSetHandle _descriptorSet,
        const eastl::span<DescriptorSetWriteInfo>& _writes)
    {
        GetImplementation(this).UpdateDescriptorSet(_descriptorSet, _writes, m_frameId);
    }

    void GraphicsContext::SetViewport(CommandListHandle _commandList, const Viewport& _viewport)
    {
        GetImplementation(this).SetViewport(
            reinterpret_cast<CommandList>(_commandList),
            _viewport);
    }

    void GraphicsContext::SetScissorsRect(CommandListHandle _commandList, const Rect& _rect)
    {
        GetImplementation(this).SetScissorsRect(
            reinterpret_cast<CommandList>(_commandList),
            _rect);
    }

    void GraphicsContext::SetIndexBuffer(CommandListHandle _commandList, const BufferView& _indexBufferView, bool _isU16)
    {
        GetImplementation(this).SetIndexBuffer(
            reinterpret_cast<CommandList>(_commandList),
            _indexBufferView,
            _isU16);
    }

    void GraphicsContext::SetVertexBuffers(CommandListHandle _commandList, const eastl::span<BufferView>& _bufferViews)
    {
        GetImplementation(this).SetVertexBuffers(
            reinterpret_cast<CommandList>(_commandList),
            _bufferViews);
    }

    void GraphicsContext::SetGraphicsPipeline(CommandListHandle _commandList, GraphicsPipelineHandle _graphicsPipeline)
    {
        GetImplementation(this).SetGraphicsPipeline(
            reinterpret_cast<CommandList>(_commandList),
            _graphicsPipeline);
    }

    void GraphicsContext::SetGraphicsPushConstant(
        CommandListHandle _commandList,
        PipelineLayoutHandle _layout,
        const eastl::span<u32>& _data,
        u32 _index,
        u32 _offset)
    {
        GetImplementation(this).SetGraphicsPushConstant(
            reinterpret_cast<CommandList>(_commandList),
            _layout,
            _data,
            _index,
            _offset);
    }

    void GraphicsContext::SetGraphicsDescriptorSets(
        CommandListHandle _commandList,
        PipelineLayoutHandle _layout,
        const eastl::span<DescriptorSetHandle>& _sets,
        const bool* _unchanged)
    {
        GetImplementation(this).SetGraphicsDescriptorSets(
            reinterpret_cast<CommandList>(_commandList),
            _layout,
            _sets,
            _unchanged,
            m_frameId);
    }

    void GraphicsContext::DrawIndexedInstanced(CommandListHandle _commandList, const DrawIndexedInstancedDesc& _desc)
    {
        GetImplementation(this).DrawIndexedInstanced(
            reinterpret_cast<CommandList>(_commandList),
            _desc);
    }
}
