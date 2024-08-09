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
}
