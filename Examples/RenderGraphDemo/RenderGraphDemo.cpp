/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>
#include <KryneEngine/Core/Window/Window.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include <KryneEngine/Core/Threads/FibersManager.hpp>
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
    std::this_thread::sleep_for(std::chrono::microseconds(50));
}

void ExecuteGBufferPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    KE_ZoneScopedFunction(__FUNCTION__);
    std::cout << "GBuffer pass" << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(700));
}

void ExecuteDeferredShadowPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    KE_ZoneScopedFunction(__FUNCTION__);
    std::cout << "Deferred shadow pass" << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(1500));
}

void ExecuteDeferredGiPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    KE_ZoneScopedFunction(__FUNCTION__);
    std::cout << "Deferred GI pass" << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(250));
}

void ExecuteDeferredShadingPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    KE_ZoneScopedFunction(__FUNCTION__);
    std::cout << "Deferred shading pass" << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(400));
}

void ExecuteSkyPass(
    RenderGraph::RenderGraph& _renderGraph,
    RenderGraph::PassExecutionData& _passExecutionData)
{
    std::cout << "Sky pass" << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(300));
}

int main()
{
    TracySetProgramName("Render graph demo");

    KE_ZoneScoped("Render graph demo");

    FibersManager fibersManager(0, AllocatorInstance());

    GraphicsCommon::ApplicationInfo appInfo {};
    appInfo.m_features.m_present = true;
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
    Window mainWindow(appInfo, AllocatorInstance());
    GraphicsContext* graphicsContext = mainWindow.GetGraphicsContext();

    RenderGraph::RenderGraph renderGraph {};

    KryneEngine::SimplePoolHandle
        swapChainTexture,
        swapChainRtv,
        frameCBuffer,
        gBufferAlbedo,
        gBufferAlbedoRtv,
        gBufferNormal,
        gBufferNormalRtv,
        gBufferDepth,
        gBufferDepthRtv,
        deferredShadow,
        deferredGi;

    {
        KE_ZoneScoped("Registration");

        swapChainTexture = renderGraph.GetRegistry().RegisterRawTexture({}, "Swapchain buffer");
        swapChainRtv = renderGraph.GetRegistry().RegisterRenderTargetView({}, swapChainTexture, "Swapchain RTV");
        frameCBuffer = renderGraph.GetRegistry().RegisterRawBuffer({}, "Frame constant buffer");
        gBufferAlbedo = renderGraph.GetRegistry().RegisterRawTexture({}, "GBuffer albedo");
        gBufferAlbedoRtv = renderGraph.GetRegistry().RegisterRenderTargetView({}, gBufferAlbedo, "GBuffer albedo RTV");
        gBufferNormal = renderGraph.GetRegistry().RegisterRawTexture({}, "GBuffer normal");
        gBufferNormalRtv = renderGraph.GetRegistry().RegisterRenderTargetView({}, gBufferNormal, "GBuffer normal RTV");
        gBufferDepth = renderGraph.GetRegistry().RegisterRawTexture({}, "GBuffer depth");
        gBufferDepthRtv = renderGraph.GetRegistry().RegisterRenderTargetView({}, gBufferDepth, "GBuffer depth RTV");
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
                    .AddColorAttachment(gBufferAlbedoRtv)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::DontCare)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                        .Done()
                    .AddColorAttachment(gBufferNormalRtv)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::DontCare)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                        .Done()
                    .SetDepthAttachment(gBufferDepthRtv)
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
                    .AddColorAttachment(swapChainRtv)
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
                    .AddColorAttachment(swapChainRtv)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::Load)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                        .Done()
                    .SetDepthAttachment(gBufferDepthRtv)
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
    while (graphicsContext->EndFrame());

    return 0;
}