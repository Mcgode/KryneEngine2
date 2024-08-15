/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#include "GraphicsContext.hpp"

#include <Graphics/Common/Window.hpp>


namespace KryneEngine
{
    GraphicsContext::GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo)
        : m_frameId(kInitialFrameId)
        , m_implementation(_appInfo, kInitialFrameId)
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
        return GetWindow()->WaitForEvents();
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
#else
        static_assert("Not yet implemented");
        return nullptr;
#endif
    }

    SamplerHandle GraphicsContext::CreateSampler(const SamplerDesc& _samplerDesc)
    {
        return m_implementation.CreateSampler(_samplerDesc);
    }

    bool GraphicsContext::DestroySampler(SamplerHandle _sampler)
    {
        return m_implementation.DestroySampler(_sampler);
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
        ShaderVisibility _visibility,
        u32 _offset)
    {
        m_implementation.SetGraphicsPushConstant(
            _commandList,
            _layout,
            _data,
            _visibility,
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
