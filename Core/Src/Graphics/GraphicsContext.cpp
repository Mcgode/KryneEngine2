/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "KryneEngine/Core/Graphics/GraphicsContext.hpp"

#include "KryneEngine/Core/Graphics/EnumHelpers.hpp"
#include "KryneEngine/Core/Graphics/ResourceViews/TextureView.hpp"
#include "KryneEngine/Core/Window/Window.hpp"

#if defined(KE_GRAPHICS_API_VK)
#   include "Graphics/Vulkan/VkGraphicsContext.hpp"
#elif defined(KE_GRAPHICS_API_DX12)
#   include "Graphics/DirectX12/Dx12GraphicsContext.hpp"
#elif defined(KE_GRAPHICS_API_MTL)
#   include "Graphics/Metal/MetalGraphicsContext.hpp"
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

    GraphicsContext* GraphicsContext::Create(
        const GraphicsCommon::ApplicationInfo& _appInfo,
        Window* _window,
        AllocatorInstance _allocator)
    {
        auto* blob = new (_allocator.Allocate<GraphicsContextBlob>()) GraphicsContextBlob {
            .m_interface = {},
            .m_implementation = Implementation(_allocator, _appInfo, _window, kInitialFrameId),
        };
        blob->m_interface.m_allocator = _allocator;
        blob->m_interface.m_frameId = kInitialFrameId;
        blob->m_interface.m_window = _window;

        return &blob->m_interface;
    }
    void GraphicsContext::Destroy(GraphicsContext* _context)
    {
        GetImplementation(_context).~Implementation();
        _context->m_allocator.deallocate(_context, sizeof(GraphicsContextBlob));
    }

    u8 GraphicsContext::GetFrameContextCount() const
    {
        return GetImplementation(this).GetFrameContextCount();
    }

    bool GraphicsContext::EndFrame()
    {
        GetImplementation(this).EndFrame(m_frameId);
        m_frameId++;
        if (m_window == nullptr)
        {
            return false;
        }
        else
        {
            return m_window->WaitForEvents();
        }
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

    bool GraphicsContext::HasDedicatedTransferQueue() const
    {
        return GetImplementation(this).HasDedicatedTransferQueue();
    }

    bool GraphicsContext::HasDedicatedComputeQueue() const
    {
        return GetImplementation(this).HasDedicatedComputeQueue();
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
        const eastl::span<const TextureMemoryFootprint>& _footprints)
    {
        return GetImplementation(this).CreateStagingBuffer(_createDesc, _footprints);
    }

    bool GraphicsContext::DestroyTexture(TextureHandle _handle)
    {
        return GetImplementation(this).DestroyTexture(_handle);
    }

    TextureViewHandle GraphicsContext::CreateTextureView(const TextureViewDesc& _viewDesc)
    {
        KE_ASSERT_MSG(!BitUtils::EnumHasAny(_viewDesc.m_accessType, TextureViewAccessType::Write)
                      || memcmp(
                         _viewDesc.m_componentsMapping,
                         (Texture4ComponentsMapping KE_DEFAULT_TEXTURE_COMPONENTS_MAPPING),
                         sizeof(Texture4ComponentsMapping)) == 0,
                      "Component remapping is not supported for write access");
        return GetImplementation(this).CreateTextureView(_viewDesc, m_frameId);
    }

    bool GraphicsContext::DestroyTextureView(TextureViewHandle _handle)
    {
        return GetImplementation(this).DestroyTextureView(_handle);
    }

    SamplerHandle GraphicsContext::CreateSampler(const SamplerDesc& _samplerDesc)
    {
        return GetImplementation(this).CreateSampler(_samplerDesc);
    }

    bool GraphicsContext::DestroySampler(SamplerHandle _sampler)
    {
        return GetImplementation(this).DestroySampler(_sampler);
    }

    BufferViewHandle GraphicsContext::CreateBufferView(const BufferViewDesc& _viewDesc)
    {
        return GetImplementation(this).CreateBufferView(_viewDesc);
    }

    bool GraphicsContext::DestroyBufferView(BufferViewHandle _handle)
    {
        return GetImplementation(this).DestroyBufferView(_handle);
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

    void GraphicsContext::EndGraphicsCommandList(CommandListHandle _commandList)
    {
        GetImplementation(this).EndGraphicsCommandList(
            reinterpret_cast<CommandList>(_commandList),
            m_frameId);
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

    void GraphicsContext::BeginComputePass(KryneEngine::CommandListHandle _commandList)
    {
        GetImplementation(this).BeginComputePass(reinterpret_cast<CommandList>(_commandList));
    }

    void GraphicsContext::EndComputePass(KryneEngine::CommandListHandle _commandList)
    {
        GetImplementation(this).EndComputePass(reinterpret_cast<CommandList>(_commandList));
    }

    void GraphicsContext::SetTextureData(
        CommandListHandle _commandList,
        BufferHandle _stagingBuffer,
        TextureHandle _dstTexture,
        const TextureMemoryFootprint& _footprint,
        const SubResourceIndexing& _subResourceIndex,
        const void* _data)
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

    bool GraphicsContext::SupportsNonGlobalBarriers()
    {
        return Implementation::SupportsNonGlobalBarriers();
    }

    void GraphicsContext::PlaceMemoryBarriers(
        CommandListHandle _commandList,
        const eastl::span<const GlobalMemoryBarrier>& _globalMemoryBarriers,
        const eastl::span<const BufferMemoryBarrier>& _bufferMemoryBarriers,
        const eastl::span<const TextureMemoryBarrier>& _textureMemoryBarriers)
    {
        return GetImplementation(this).PlaceMemoryBarriers(
            reinterpret_cast<CommandList>(_commandList),
            _globalMemoryBarriers,
            _bufferMemoryBarriers,
            _textureMemoryBarriers);
    }

    bool GraphicsContext::RenderPassNeedsUsageDeclaration()
    {
        return Implementation::RenderPassNeedsUsageDeclaration();
    }
    bool GraphicsContext::ComputePassNeedsUsageDeclaration()
    {
        return Implementation::ComputePassNeedsUsageDeclaration();
    }

    void GraphicsContext::DeclarePassTextureViewUsage(
        CommandListHandle _commandList,
        const eastl::span<const TextureViewHandle>& _textures,
        TextureViewAccessType _accessType)
    {
        GetImplementation(this).DeclarePassTextureViewUsage(
            reinterpret_cast<CommandList>(_commandList),
            _textures,
            _accessType);
    }

    void GraphicsContext::DeclarePassBufferViewUsage(
        KryneEngine::CommandListHandle _commandList,
        const eastl::span<const BufferViewHandle>& _buffers,
        BufferViewAccessType _accessType)
    {
        GetImplementation(this).DeclarePassBufferViewUsage(
            reinterpret_cast<CommandList>(_commandList),
            _buffers,
            _accessType);
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

    ComputePipelineHandle GraphicsContext::CreateComputePipeline(const ComputePipelineDesc& _desc)
    {
        return GetImplementation(this).CreateComputePipeline(_desc);
    }

    bool GraphicsContext::DestroyComputePipeline(ComputePipelineHandle _pipeline)
    {
        return GetImplementation(this).DestroyComputePipeline(_pipeline);
    }

    void GraphicsContext::UpdateDescriptorSet(
        DescriptorSetHandle _descriptorSet,
        const eastl::span<const DescriptorSetWriteInfo>& _writes)
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

    void GraphicsContext::SetIndexBuffer(CommandListHandle _commandList, const BufferSpan& _indexBufferView, bool _isU16)
    {
        GetImplementation(this).SetIndexBuffer(
            reinterpret_cast<CommandList>(_commandList),
            _indexBufferView,
            _isU16);
    }

    void GraphicsContext::SetVertexBuffers(CommandListHandle _commandList, const eastl::span<const BufferSpan>& _bufferViews)
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
        const eastl::span<const u32>& _data,
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
        const eastl::span<const DescriptorSetHandle>& _sets,
        const bool* _unchanged)
    {
        GetImplementation(this).SetGraphicsDescriptorSets(
            reinterpret_cast<CommandList>(_commandList),
            _layout,
            _sets,
            _unchanged,
            m_frameId);
    }

    void GraphicsContext::DrawInstanced(CommandListHandle _commandList, const DrawInstancedDesc& _desc)
    {
        GetImplementation(this).DrawInstanced(
            reinterpret_cast<CommandList>(_commandList),
            _desc);
    }

    void GraphicsContext::DrawIndexedInstanced(CommandListHandle _commandList, const DrawIndexedInstancedDesc& _desc)
    {
        GetImplementation(this).DrawIndexedInstanced(
            reinterpret_cast<CommandList>(_commandList),
            _desc);
    }

    void GraphicsContext::SetComputePipeline(CommandListHandle _commandList, ComputePipelineHandle _pipeline)
    {
        GetImplementation(this).SetComputePipeline(
            reinterpret_cast<CommandList>(_commandList),
            _pipeline);
    }

    void GraphicsContext::SetComputeDescriptorSets(
        CommandListHandle _commandList,
        PipelineLayoutHandle _layout,
        eastl::span<const DescriptorSetHandle> _sets,
        u32 _offset)
    {
        GetImplementation(this).SetComputeDescriptorSets(
            reinterpret_cast<CommandList>(_commandList),
            _layout,
            _sets,
            _offset,
            m_frameId);
    }

    void GraphicsContext::SetComputePushConstant(
        CommandListHandle _commandList,
        PipelineLayoutHandle _layout,
        eastl::span<const u32> _data)
    {
        GetImplementation(this).SetComputePushConstant(
            reinterpret_cast<CommandList>(_commandList),
            _layout,
            _data);
    }

    void GraphicsContext::Dispatch(CommandListHandle _commandList, uint3 _threadGroupCount, uint3 _threadGroupSize)
    {
        GetImplementation(this).Dispatch(
            reinterpret_cast<CommandList>(_commandList),
            _threadGroupCount,
            _threadGroupSize);
    }
}
