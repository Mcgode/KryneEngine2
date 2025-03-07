/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>
#include <KryneEngine/Core/Window/Window.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include <KryneEngine/Core/Threads/FibersManager.hpp>
#include <KryneEngine/Modules/ImGui/Context.hpp>
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

    AllocatorInstance allocator = AllocatorInstance();

    FibersManager fibersManager(0, allocator);

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
    Window mainWindow(appInfo, allocator);
    GraphicsContext* graphicsContext = mainWindow.GetGraphicsContext();

    Modules::ImGui::Context* imGuiContext = nullptr;

    RenderGraph::RenderGraph renderGraph {};

    KryneEngine::SimplePoolHandle
        frameCBuffer,
        gBufferAlbedo,
        gBufferAlbedoRtv,
        gBufferNormal,
        gBufferNormalRtv,
        gBufferDepth,
        gBufferDepthRtv,
        deferredShadow,
        deferredGi;

    DynamicArray<SimplePoolHandle> swapChainTextures(allocator, graphicsContext->GetFrameContextCount());
    DynamicArray<SimplePoolHandle> swapChainRtvs(allocator, graphicsContext->GetFrameContextCount());

    {
        KE_ZoneScoped("Registration");

        for (auto i = 0u; i < graphicsContext->GetFrameContextCount(); i++)
        {
            eastl::string name;

            swapChainTextures[i] = renderGraph.GetRegistry().RegisterRawTexture(
                graphicsContext->GetPresentTexture(i),
                name.sprintf("Swapchain buffer %u", i));
            swapChainRtvs[i] = renderGraph.GetRegistry().RegisterRenderTargetView(
                graphicsContext->GetPresentRenderTargetView(i),
                swapChainTextures[i],
                name.sprintf("Swapchain RTV %u", i));
        }
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
        if (imGuiContext == nullptr)
        {
            KE_ZoneScoped("Init ImGui context");

            // Even if it's a dummy pass, the generated render pass should match signature with the one in the render
            // graph for the ImGui pass, so it will be reused there.

            RenderGraph::PassDeclaration imguiDummyPass(KryneEngine::Modules::RenderGraph::PassType::Render, 0);
            RenderGraph::PassDeclarationBuilder(imguiDummyPass, nullptr)
                .AddColorAttachment(swapChainRtvs[0])
                    .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::Load)
                    .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                    .Done();

            imGuiContext = allocator.New<Modules::ImGui::Context>(
                &mainWindow,
                renderGraph.FetchRenderPass(*graphicsContext, imguiDummyPass),
                allocator);
        }

        imGuiContext->NewFrame(&mainWindow);

        RenderGraph::Builder& builder = renderGraph.BeginFrame(*graphicsContext);

        SimplePoolHandle swapChainTexture = swapChainTextures[graphicsContext->GetCurrentPresentImageIndex()];
        SimplePoolHandle swapChainRtv = swapChainRtvs[graphicsContext->GetCurrentPresentImageIndex()];

        {
            KE_ZoneScoped("Build render graph");

            const auto transferExecuteFunction = [&](RenderGraph::RenderGraph& _renderGraph, RenderGraph::PassExecutionData _passData)
            {
                ExecuteUploadData(_renderGraph, _passData);
                imGuiContext->PrepareToRenderFrame(graphicsContext, _passData.m_commandList);
            };

            builder
                .DeclarePass(RenderGraph::PassType::Transfer)
                    .SetName("Upload data")
                    .SetExecuteFunction(transferExecuteFunction)
                    .WriteDependency(frameCBuffer)
                    .Done()
                .DeclarePass(RenderGraph::PassType::Compute) // TODO: fix when render targets are created
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
                .DeclarePass(KryneEngine::Modules::RenderGraph::PassType::Compute) // TODO: fix when render targets are created
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
            KE_ZoneScoped("Build ImGui pass");

            const auto executeFunction = [&](
                                             RenderGraph::RenderGraph& _renderGraph,
                                             RenderGraph::PassExecutionData& _passData)
            {
                imGuiContext->RenderFrame(graphicsContext, _passData.m_commandList);
            };

            builder
                .DeclarePass(RenderGraph::PassType::Render)
                .SetName("ImGui pass")
                .SetExecuteFunction(executeFunction)
                .AddColorAttachment(swapChainRtv)
                    .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::Load)
                    .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                    .Done();
        }

        {
            KE_ZoneScoped("Execute render graph");

            renderGraph.SubmitFrame(*graphicsContext, fibersManager);
        }
    }
    while (graphicsContext->EndFrame());

    if (imGuiContext)
    {
        allocator.Delete(imGuiContext);
    }

    return 0;
}