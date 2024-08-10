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

    ShaderModuleHandle GraphicsContext::RegisterShaderModule(void* _bytecodeData, u64 _bytecodeSize)
    {
        return m_implementation.RegisterShaderModule(_bytecodeData, _bytecodeSize);
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
}
