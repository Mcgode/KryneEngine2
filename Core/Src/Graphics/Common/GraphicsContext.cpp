/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "KryneEngine/Core/Graphics/Common/GraphicsContext.hpp"

#include "KryneEngine/Core/Graphics/Common/EnumHelpers.hpp"
#include "KryneEngine/Core/Window/Window.hpp"


namespace KryneEngine
{
    GraphicsContext::GraphicsContext(const GraphicsCommon::ApplicationInfo& _appInfo, Window* _window)
        : m_frameId(kInitialFrameId)
        , m_implementation(_appInfo, _window, kInitialFrameId)
        , m_window(_window)
    {
    }

    GraphicsContext::~GraphicsContext()
    {
        WaitForLastFrame();
    }

    bool GraphicsContext::EndFrame()
    {
        m_implementation.EndFrame(m_frameId);
        m_frameId++;
        return m_window->WaitForEvents();
    }

    void GraphicsContext::WaitForLastFrame() const
    {
        m_implementation.WaitForFrame(m_frameId - 1);
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

        return m_implementation.CreateTexture(_createDesc);
    }

    SamplerHandle GraphicsContext::CreateSampler(const SamplerDesc& _samplerDesc)
    {
        return m_implementation.CreateSampler(_samplerDesc);
    }

    bool GraphicsContext::DestroySampler(SamplerHandle _sampler)
    {
        return m_implementation.DestroySampler(_sampler);
    }

    TextureHandle GraphicsContext::GetPresentTexture(u8 _swapChainIndex)
    {
        return m_implementation.GetPresentTexture(_swapChainIndex);
    }

    void GraphicsContext::DeclarePassTextureSrvUsage(
        CommandList _commandList,
        const eastl::span<TextureSrvHandle>& _textures)
    {
        m_implementation.DeclarePassTextureSrvUsage(_commandList, _textures);
    }

    ShaderModuleHandle GraphicsContext::RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize)
    {
        return m_implementation.RegisterShaderModule(_bytecodeData, _bytecodeSize);
    }

    DescriptorSetLayoutHandle GraphicsContext::CreateDescriptorSetLayout(
        const DescriptorSetDesc& _desc,
        u32* _bindingIndices)
    {
        return m_implementation.CreateDescriptorSetLayout(_desc, _bindingIndices);
    }

    DescriptorSetHandle GraphicsContext::CreateDescriptorSet(DescriptorSetLayoutHandle _layout)
    {
        return m_implementation.CreateDescriptorSet(_layout);
    }

    PipelineLayoutHandle GraphicsContext::CreatePipelineLayout(const PipelineLayoutDesc& _desc)
    {
        return m_implementation.CreatePipelineLayout(_desc);
    }

    GraphicsPipelineHandle GraphicsContext::CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc)
    {
        return m_implementation.CreateGraphicsPipeline(_desc);
    }

    bool GraphicsContext::DestroyGraphicsPipeline(GraphicsPipelineHandle _pipeline)
    {
        return m_implementation.DestroyGraphicsPipeline(_pipeline);
    }

    bool GraphicsContext::DestroyPipelineLayout(PipelineLayoutHandle _layout)
    {
        return m_implementation.DestroyPipelineLayout(_layout);
    }

    bool GraphicsContext::DestroyDescriptorSet(DescriptorSetHandle _set)
    {
        return m_implementation.DestroyDescriptorSet(_set);
    }

    bool GraphicsContext::DestroyDescriptorSetLayout(DescriptorSetLayoutHandle _layout)
    {
        return m_implementation.DestroyDescriptorSetLayout(_layout);
    }

    bool GraphicsContext::FreeShaderModule(ShaderModuleHandle _module)
    {
        return m_implementation.FreeShaderModule(_module);
    }

    void GraphicsContext::UpdateDescriptorSet(
        DescriptorSetHandle _descriptorSet,
        const eastl::span<DescriptorSetWriteInfo>& _writes)
    {
        m_implementation.UpdateDescriptorSet(_descriptorSet, _writes, m_frameId);
    }

    void GraphicsContext::SetViewport(CommandList _commandList, const Viewport& _viewport)
    {
        m_implementation.SetViewport(_commandList, _viewport);
    }

    void GraphicsContext::SetScissorsRect(CommandList _commandList, const Rect& _rect)
    {
        m_implementation.SetScissorsRect(_commandList, _rect);
    }

    void GraphicsContext::SetIndexBuffer(CommandList _commandList, const BufferView& _indexBufferView, bool _isU16)
    {
        m_implementation.SetIndexBuffer(_commandList, _indexBufferView, _isU16);
    }

    void GraphicsContext::SetVertexBuffers(CommandList _commandList, const eastl::span<BufferView>& _bufferViews)
    {
        m_implementation.SetVertexBuffers(_commandList, _bufferViews);
    }

    void GraphicsContext::SetGraphicsPipeline(CommandList _commandList, GraphicsPipelineHandle _graphicsPipeline)
    {
        m_implementation.SetGraphicsPipeline(_commandList, _graphicsPipeline);
    }

    void GraphicsContext::SetGraphicsPushConstant(
        CommandList _commandList,
        PipelineLayoutHandle _layout,
        const eastl::span<u32>& _data,
        u32 _index,
        u32 _offset)
    {
        m_implementation.SetGraphicsPushConstant(
            _commandList,
            _layout,
            _data,
            _index,
            _offset);
    }

    void GraphicsContext::SetGraphicsDescriptorSets(
        CommandList _commandList,
        PipelineLayoutHandle _layout,
        const eastl::span<DescriptorSetHandle>& _sets,
        const bool* _unchanged)
    {
        m_implementation.SetGraphicsDescriptorSets(
            _commandList,
            _layout,
            _sets,
            _unchanged,
            m_frameId);
    }

    void GraphicsContext::DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc)
    {
        m_implementation.DrawIndexedInstanced(_commandList, _desc);
    }
}
