/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include "KryneEngine/Core/Threads/FibersManager.hpp"
#include <KryneEngine/Modules/RenderGraph/Builder.hpp>
#include <KryneEngine/Modules/RenderGraph/Registry.hpp>
#include <KryneEngine/Modules/RenderGraph/RenderGraph.hpp>
#include <iostream>

using namespace KryneEngine;
using namespace KryneEngine::Modules;

void ExecuteUploadData(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    KE_ZoneScopedFunction(__FUNCTION__);
    std::cout << "Uploading constant buffer" << std::endl;
}

void ExecuteGBufferPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    KE_ZoneScopedFunction(__FUNCTION__);
    std::cout << "GBuffer pass" << std::endl;
}

void ExecuteDeferredShadowPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    KE_ZoneScopedFunction(__FUNCTION__);
    std::cout << "Deferred shadow pass" << std::endl;
}

void ExecuteDeferredGiPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    KE_ZoneScopedFunction(__FUNCTION__);
    std::cout << "Deferred GI pass" << std::endl;
}

void ExecuteDeferredShadingPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    KE_ZoneScopedFunction(__FUNCTION__);
    std::cout << "Deferred shading pass" << std::endl;
}

void ExecuteSkyPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    std::cout << "Sky pass" << std::endl;
}

int main()
{
    TracySetProgramName("Render graph demo");

    KE_ZoneScoped("Render graph demo");

    FibersManager fibersManager(0, AllocatorInstance());

    GraphicsCommon::ApplicationInfo appInfo {};
    appInfo.m_features.m_present = false;
    appInfo.m_applicationName = "Render graph demo - Kryne Engine 2";
#if defined(KE_GRAPHICS_API_VK)
    appInfo.m_api = GraphicsCommon::Api::Vulkan_1_3;
    appInfo.m_applicationName += " - Vulkan";
#elif defined(KE_GRAPHICS_API_DX12)
    appInfo.m_api = KryneEngine::GraphicsCommon::Api::DirectX12_1;
    appInfo.m_applicationName += " - DirectX 12";
#elif defined(KE_GRAPHICS_API_MTL)
    appInfo.m_api = GraphicsCommon::Api::Metal_3;
    appInfo.m_applicationName += " - Metal";
#endif
    auto* graphicsContext = GraphicsContext::Create(appInfo, nullptr, AllocatorInstance());

    RenderGraph::RenderGraph renderGraph {};

    KryneEngine::SimplePoolHandle
        swapChainTexture,
        frameCBuffer,
        gBufferAlbedo,
        gBufferNormal,
        gBufferDepth,
        deferredShadow,
        deferredGi;

    {
        KE_ZoneScoped("Registration");

        swapChainTexture = renderGraph.GetRegistry().RegisterRawTexture({}, "Swapchain buffer");
        frameCBuffer = renderGraph.GetRegistry().RegisterRawBuffer({}, "Frame constant buffer");
        gBufferAlbedo = renderGraph.GetRegistry().RegisterRawTexture({}, "GBuffer albedo");
        gBufferNormal = renderGraph.GetRegistry().RegisterRawTexture({}, "GBuffer normal");
        gBufferDepth = renderGraph.GetRegistry().RegisterRawTexture({}, "GBuffer depth");
        deferredShadow = renderGraph.GetRegistry().RegisterRawTexture({}, "Deferred shadow");
        deferredGi = renderGraph.GetRegistry().RegisterRawTexture({}, "Deferred GI");
    }

    do
    {
        RenderGraph::Builder& builder = renderGraph.BeginFrame(*graphicsContext);

        {
            KE_ZoneScoped("Build render graph");

            builder
                .DeclarePass(RenderGraph::PassType::Transfer)
                    .SetName("Upload data")
                    .SetExecuteFunction(ExecuteUploadData)
                    .WriteDependency(frameCBuffer)
                    .Done()
                .DeclarePass(RenderGraph::PassType::Render)
                    .SetName("GBuffer pass")
                    .SetExecuteFunction(ExecuteGBufferPass)
                    .AddColorAttachment(gBufferAlbedo)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::DontCare)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                        .Done()
                    .AddColorAttachment(gBufferNormal)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::DontCare)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                        .Done()
                    .SetDepthAttachment(gBufferDepth)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::Clear)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                        .SetClearDepthStencil(0.f, 0)
                        .Done()
                    .ReadDependency(frameCBuffer)
                    .Done()
                .DeclarePass(RenderGraph::PassType::Compute)
                    .SetName("Deferred shadow pass")
                    .SetExecuteFunction(ExecuteDeferredShadowPass)
                    .ReadDependency(frameCBuffer)
                    .ReadDependency(gBufferDepth)
                    .WriteDependency(deferredShadow)
                    .Done()
                .DeclarePass(RenderGraph::PassType::Compute)
                    .SetName("Deferred 'GI' pass")
                    .SetExecuteFunction(ExecuteDeferredGiPass)
                    .ReadDependency(frameCBuffer)
                    .ReadDependency(gBufferAlbedo)
                    .ReadDependency(gBufferNormal)
                    .ReadDependency(gBufferDepth)
                    .WriteDependency(deferredGi)
                    .Done()
                .DeclarePass(KryneEngine::Modules::RenderGraph::PassType::Render)
                    .SetName("Deferred shading pass")
                    .SetExecuteFunction(ExecuteDeferredShadingPass)
                    .AddColorAttachment(swapChainTexture)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::DontCare)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                        .Done()
                    .ReadDependency(frameCBuffer)
                    .ReadDependency(gBufferAlbedo)
                    .ReadDependency(gBufferNormal)
                    .ReadDependency(gBufferDepth)
                    .ReadDependency(deferredShadow)
                    .ReadDependency(deferredGi)
                    .Done()
                .DeclarePass(KryneEngine::Modules::RenderGraph::PassType::Render)
                    .SetName("Sky pass")
                    .SetExecuteFunction(ExecuteSkyPass)
                    .AddColorAttachment(swapChainTexture)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::Load)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                        .Done()
                    .SetDepthAttachment(gBufferDepth)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::Load)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::DontCare)
                        .Done()
                    .ReadDependency(frameCBuffer)
                    .Done()
                .DeclareTargetResource(swapChainTexture);
        }

        {
            KE_ZoneScoped("Execute render graph");

            renderGraph.SubmitFrame(*graphicsContext, fibersManager);
        }
    }
    while (!graphicsContext->EndFrame());

    return 0;
}