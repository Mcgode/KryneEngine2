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

    DescriptorSetHandle GraphicsContext::CreateDescriptorSet(const DescriptorSetDesc& _desc, u32* _bindingIndices)
    {
        return m_implementation.CreateDescriptorSet(_desc, _bindingIndices);
    }

    PipelineLayoutHandle GraphicsContext::CreatePipelineLayout(const PipelineLayoutDesc& _desc)
    {
        return m_implementation.CreatePipelineLayout(_desc);
    }

    GraphicsPipelineHandle GraphicsContext::CreateGraphicsPipeline(const GraphicsPipelineDesc& _desc)
    {
        return m_implementation.CreateGraphicsPipeline(_desc);
    }

    void GraphicsContext::SetViewport(CommandList _commandList, const Viewport& _viewport)
    {
        m_implementation.SetViewport(_commandList, _viewport);
    }

    void GraphicsContext::SetScissorsRect(CommandList _commandList, const Rect& _rect)
    {
        m_implementation.SetScissorsRect(_commandList, _rect);
    }

    void GraphicsContext::SetIndexBuffer(CommandList _commandList, BufferHandle _indexBuffer, u64 _bufferSize, bool _isU16)
    {
        m_implementation.SetIndexBuffer(_commandList, _indexBuffer, _bufferSize, _isU16);
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
        u32 _index,
        const eastl::span<u32>& _data,
        u32 _offset)
    {
        m_implementation.SetGraphicsPushConstant(_commandList, _index, _data, _offset);
    }

    void GraphicsContext::DrawIndexedInstanced(CommandList _commandList, const DrawIndexedInstancedDesc& _desc)
    {
        m_implementation.DrawIndexedInstanced(_commandList, _desc);
    }
}
