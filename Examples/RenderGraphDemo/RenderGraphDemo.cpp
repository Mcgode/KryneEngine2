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
#include <KryneEngine/Modules/RenderGraph/Descriptors/RenderTargetViewDesc.hpp>
#include <KryneEngine/Modules/RenderGraph/Registry.hpp>
#include <KryneEngine/Modules/RenderGraph/RenderGraph.hpp>
#include <iostream>

#include "Scene/SceneManager.hpp"

using namespace KryneEngine;
using namespace KryneEngine::Modules;
using namespace KryneEngine::Samples::RenderGraphDemo;

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
    SceneManager sceneManager(allocator, mainWindow, renderGraph.GetRegistry());

    KryneEngine::SimplePoolHandle
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

        const uint3 dimensions(appInfo.m_displayOptions.m_width, appInfo.m_displayOptions.m_height, 1);

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

        gBufferAlbedo = renderGraph.GetRegistry().CreateRawTexture(
            graphicsContext,
            {
                .m_desc = {
                    .m_dimensions = dimensions,
                    .m_format = KryneEngine::TextureFormat::RGBA8_UNorm,
#if !defined(KE_FINAL)
                    .m_debugName = "GBuffer albedo",
#endif
                },
                .m_memoryUsage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::ColorTargetImage | MemoryUsage::ReadImage,
            });
        gBufferAlbedoRtv = renderGraph.GetRegistry().CreateRenderTargetView(
            graphicsContext,
            RenderGraph::RenderTargetViewDesc {
                .m_textureResource = gBufferAlbedo,
                .m_format = KryneEngine::TextureFormat::RGBA8_UNorm,
            },
            "GBuffer albedo RTV");

        gBufferNormal = renderGraph.GetRegistry().CreateRawTexture(
            graphicsContext,
            {
                .m_desc = {
                    .m_dimensions = dimensions,
                    .m_format = KryneEngine::TextureFormat::RGBA8_UNorm, // TODO: Implement RGB10A2 format support
#if !defined(KE_FINAL)
                    .m_debugName = "GBuffer normal",
#endif
                },
                .m_memoryUsage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::ColorTargetImage | MemoryUsage::ReadImage,
            });
        gBufferNormalRtv = renderGraph.GetRegistry().CreateRenderTargetView(
            graphicsContext,
            RenderGraph::RenderTargetViewDesc {
                .m_textureResource = gBufferNormal,
                .m_format = KryneEngine::TextureFormat::RGBA8_UNorm, // TODO: Implement RGB10A2 format support
            },
            "GBuffer normal RTV");

        gBufferDepth = renderGraph.GetRegistry().CreateRawTexture(
            graphicsContext,
            {
                .m_desc = {
                    .m_dimensions = dimensions,
                    .m_format = KryneEngine::TextureFormat::D32F,
                    .m_planes = TexturePlane::Depth,
#if !defined(KE_FINAL)
                    .m_debugName = "GBuffer depth"
#endif
                },
                .m_memoryUsage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::DepthStencilTargetImage | MemoryUsage::ReadImage,
            });
        gBufferDepthRtv = renderGraph.GetRegistry().CreateRenderTargetView(
            graphicsContext,
            RenderGraph::RenderTargetViewDesc {
                .m_textureResource = gBufferDepth,
                .m_format = KryneEngine::TextureFormat::D32F,
                .m_plane = TexturePlane::Depth,
            },
            "GBuffer depth RTV");

        deferredShadow = renderGraph.GetRegistry().CreateRawTexture(
            graphicsContext,
            {
                .m_desc = {
                    .m_dimensions = dimensions,
                    .m_format = KryneEngine::TextureFormat::R8_UNorm,
#if !defined(KE_FINAL)
                    .m_debugName = "Deferred shadow",
#endif
                },
                .m_memoryUsage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::ReadWriteImage,
            });
        deferredGi = renderGraph.GetRegistry().CreateRawTexture(
            graphicsContext,
            {
                .m_desc = {
                    .m_dimensions = dimensions,
                    .m_format = KryneEngine::TextureFormat::RGBA32_Float, // TODO: Implement RGBA16F support
#if !defined(KE_FINAL)
                    .m_debugName = "Deferred GI",
#endif
                },
                .m_memoryUsage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::ReadWriteImage,
            });
    }

    // Init scene PSOs
    {
        RenderGraph::PassDeclaration gBufferDummyPass(KryneEngine::Modules::RenderGraph::PassType::Render, 0);
        RenderGraph::PassDeclarationBuilder(gBufferDummyPass, nullptr)
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
                .Done();
        gBufferDummyPass.m_colorAttachments[0].m_layoutBefore = TextureLayout::ColorAttachment;
        gBufferDummyPass.m_colorAttachments[0].m_layoutAfter = TextureLayout::ShaderResource;
        gBufferDummyPass.m_colorAttachments[1].m_layoutBefore = TextureLayout::ColorAttachment;
        gBufferDummyPass.m_colorAttachments[1].m_layoutAfter = TextureLayout::ShaderResource;
        gBufferDummyPass.m_depthAttachment.value().m_layoutBefore = TextureLayout::DepthStencilAttachment;
        gBufferDummyPass.m_depthAttachment.value().m_layoutAfter = TextureLayout::ShaderResource;

        sceneManager.PreparePsos(
            graphicsContext,
            renderGraph.FetchRenderPass(*graphicsContext, gBufferDummyPass));
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
            imguiDummyPass.m_colorAttachments[0].m_layoutBefore = TextureLayout::ColorAttachment;
            imguiDummyPass.m_colorAttachments[0].m_layoutAfter = TextureLayout::ColorAttachment;

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

            sceneManager.DeclareDataTransferPass(graphicsContext, builder, imGuiContext);

            const RenderGraph::Dependency frameCBufferReadDep {
                .m_resource = sceneManager.GetSceneConstantsCbv(),
                .m_targetAccessFlags = BarrierAccessFlags::ConstantBuffer,
            };

            builder
                .DeclarePass(RenderGraph::PassType::Render)
                    .SetName("GBuffer pass")
                    .SetExecuteFunction([&sceneManager](const auto& _, const auto& _passData)
                        {
                            KE_ZoneScoped("Render GBuffer");
                            sceneManager.RenderGBuffer(_passData.m_graphicsContext, _passData.m_commandList);
                        })
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
                    .ReadDependency(frameCBufferReadDep)
                    .Done()
                .DeclarePass(RenderGraph::PassType::Compute)
                    .SetName("Deferred shadow pass")
                    .SetExecuteFunction(ExecuteDeferredShadowPass)
                    .ReadDependency(frameCBufferReadDep)
                    .ReadDependency({
                        .m_resource = gBufferDepth,
                        .m_targetSyncStage = BarrierSyncStageFlags::ComputeShading,
                        .m_targetAccessFlags = BarrierAccessFlags::ShaderResource,
                        .m_targetLayout = TextureLayout::ShaderResource,
                        .m_planes = TexturePlane::Depth,
                    })
                    .WriteDependency({
                        .m_resource = deferredShadow,
                        .m_targetSyncStage = BarrierSyncStageFlags::ComputeShading,
                        .m_targetAccessFlags = BarrierAccessFlags::UnorderedAccess,
                        .m_targetLayout = TextureLayout::UnorderedAccess,
                    })
                    .Done()
                .DeclarePass(RenderGraph::PassType::Compute)
                    .SetName("Deferred 'GI' pass")
                    .SetExecuteFunction(ExecuteDeferredGiPass)
                    .ReadDependency(frameCBufferReadDep)
                    .ReadDependency({
                        .m_resource = gBufferAlbedo,
                        .m_targetSyncStage = BarrierSyncStageFlags::ComputeShading,
                        .m_targetAccessFlags = BarrierAccessFlags::ShaderResource,
                        .m_targetLayout = TextureLayout::ShaderResource,
                    })
                    .ReadDependency({
                        .m_resource = gBufferNormal,
                        .m_targetSyncStage = BarrierSyncStageFlags::ComputeShading,
                        .m_targetAccessFlags = BarrierAccessFlags::ShaderResource,
                        .m_targetLayout = TextureLayout::ShaderResource,
                    })
                    .ReadDependency({
                        .m_resource = gBufferDepth,
                        .m_targetSyncStage = BarrierSyncStageFlags::ComputeShading,
                        .m_targetAccessFlags = BarrierAccessFlags::ShaderResource,
                        .m_targetLayout = TextureLayout::ShaderResource,
                        .m_planes = TexturePlane::Depth,
                    })
                    .WriteDependency({
                        .m_resource = deferredGi,
                        .m_targetSyncStage = BarrierSyncStageFlags::ComputeShading,
                        .m_targetAccessFlags = BarrierAccessFlags::UnorderedAccess,
                        .m_targetLayout = TextureLayout::UnorderedAccess,
                    })
                    .Done()
                .DeclarePass(KryneEngine::Modules::RenderGraph::PassType::Render)
                    .SetName("Deferred shading pass")
                    .SetExecuteFunction(ExecuteDeferredShadingPass)
                    .AddColorAttachment(swapChainRtv)
                        .SetLoadOperation(RenderPassDesc::Attachment::LoadOperation::DontCare)
                        .SetStoreOperation(RenderPassDesc::Attachment::StoreOperation::Store)
                        .Done()
                    .ReadDependency({
                        .m_resource = gBufferAlbedo,
                        .m_targetSyncStage = BarrierSyncStageFlags::FragmentShading,
                        .m_targetAccessFlags = BarrierAccessFlags::ShaderResource,
                        .m_targetLayout = TextureLayout::ShaderResource,
                    })
                    .ReadDependency({
                        .m_resource = gBufferNormal,
                        .m_targetSyncStage = BarrierSyncStageFlags::FragmentShading,
                        .m_targetAccessFlags = BarrierAccessFlags::ShaderResource,
                        .m_targetLayout = TextureLayout::ShaderResource,
                    })
                    .ReadDependency({
                        .m_resource = gBufferDepth,
                        .m_targetSyncStage = BarrierSyncStageFlags::FragmentShading,
                        .m_targetAccessFlags = BarrierAccessFlags::ShaderResource,
                        .m_targetLayout = TextureLayout::ShaderResource,
                        .m_planes = TexturePlane::Depth,
                    })
                    .ReadDependency({
                        .m_resource = deferredShadow,
                        .m_targetSyncStage = BarrierSyncStageFlags::FragmentShading,
                        .m_targetAccessFlags = BarrierAccessFlags::ShaderResource,
                        .m_targetLayout = TextureLayout::ShaderResource,
                    })
                    .ReadDependency({
                        .m_resource = deferredGi,
                        .m_targetSyncStage = BarrierSyncStageFlags::FragmentShading,
                        .m_targetAccessFlags = BarrierAccessFlags::ShaderResource,
                        .m_targetLayout = TextureLayout::ShaderResource,
                    })
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
                    .ReadDependency(frameCBufferReadDep)
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
            KE_ZoneScoped("Process scene");
            sceneManager.Process(graphicsContext);
        }

        {
            KE_ZoneScoped("Execute render graph");

            renderGraph.SubmitFrame(*graphicsContext, nullptr);
        }
    }
    while (graphicsContext->EndFrame());

    if (imGuiContext)
    {
        imGuiContext->Shutdown(&mainWindow);
        allocator.Delete(imGuiContext);
    }

    return 0;
}