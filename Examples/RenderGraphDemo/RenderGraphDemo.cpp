/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include <Profiling/TracyHeader.hpp>
#include <RenderGraph/Declarations/PassDeclaration.hpp>
#include <RenderGraph/RenderGraph.hpp>
#include <RenderGraph/Registry.hpp>
#include <RenderGraph/Builder.hpp>

#include<iostream>

using namespace KryneEngine::Modules;

void ExecuteUploadConstantBuffer(
    [[maybe_unused]] RenderGraph::RenderGraph& _renderGraph,
    [[maybe_unused]] RenderGraph::PassExecutionData& _passExecutionData)
{
    std::cout << "Uploading constant buffer" << std::endl;
}

int main()
{
    TracySetProgramName("Render graph demo");

    RenderGraph::RenderGraph renderGraph {};
    RenderGraph::Builder& builder = renderGraph.BeginFrame();

    KryneEngine::SimplePoolHandle
        swapChainTexture,
        csTexture,
        texGenBuffer,
        frameCBuffer,
        lightsBuffer,
        lightingAtlas,
        lightingAtlasSrv;

    {
        KE_ZoneScoped("Registration");

        swapChainTexture = renderGraph.GetRegistry().RegisterRawTexture({}, "Swapchain buffer");
        csTexture = renderGraph.GetRegistry().RegisterRawTexture({}, "Compute shader texture");
        texGenBuffer = renderGraph.GetRegistry().RegisterRawBuffer({}, "Texture generation buffer");
        frameCBuffer = renderGraph.GetRegistry().RegisterRawBuffer({}, "Frame constant buffer");
        lightsBuffer = renderGraph.GetRegistry().RegisterRawBuffer({}, "Lights buffer");
        lightingAtlas = renderGraph.GetRegistry().RegisterRawTexture({}, "Lighting atlas");
        lightingAtlasSrv = renderGraph.GetRegistry().RegisterTextureSrv({}, lightingAtlas, "Lighting atlas SRV");
    }

    {
        KE_ZoneScoped("Build render graph");

        builder
            .DeclareTargetResource(swapChainTexture)
            .DeclarePass(RenderGraph::PassType::Transfer)
                .SetName("Upload constant buffer")
                .SetExecuteFunction(ExecuteUploadConstantBuffer)
                .WriteDependency(frameCBuffer)
                .Done()
            .DeclarePass(RenderGraph::PassType::Compute)
                .SetName("Recompute generative buffer")
                .SetExecuteFunction([](RenderGraph::RenderGraph&, RenderGraph::PassExecutionData&){})
                .ReadDependency(texGenBuffer)
                .WriteDependency(texGenBuffer)
                .Done()
            .DeclarePass(RenderGraph::PassType::Compute)
                .SetName("Texture generation")
                .SetExecuteFunction( [](RenderGraph::RenderGraph&, RenderGraph::PassExecutionData&){})
                .ReadDependency(texGenBuffer)
                .ReadDependency(frameCBuffer)
                .WriteDependency(csTexture)
                .Done()
            .DeclarePass(RenderGraph::PassType::Compute)
                .SetName("Light dispatch")
                .SetExecuteFunction([](RenderGraph::RenderGraph&, RenderGraph::PassExecutionData&){})
                .ReadDependency(frameCBuffer)
                .WriteDependency(lightsBuffer)
                .Done()
            .DeclarePass(RenderGraph::PassType::Render)
                .SetName("Light atlas draw")
                .SetExecuteFunction( [](RenderGraph::RenderGraph&, RenderGraph::PassExecutionData&){})
                .AddColorAttachment(lightingAtlas)
                    .SetLoadOperation(KryneEngine::RenderPassDesc::Attachment::LoadOperation::DontCare)
                    .Done()
                .ReadDependency(lightsBuffer)
                .Done()
            .DeclarePass(RenderGraph::PassType::Render)
                .SetName("Final draw")
                .SetExecuteFunction( [](RenderGraph::RenderGraph&, RenderGraph::PassExecutionData&){})
                .AddColorAttachment(swapChainTexture)
                    .SetLoadOperation(KryneEngine::RenderPassDesc::Attachment::LoadOperation::Clear)
                    .SetStoreOperation(KryneEngine::RenderPassDesc::Attachment::StoreOperation::Store)
                    .SetClearColor({ 0, 1, 1, 1 })
                    .Done()
                .ReadDependency(frameCBuffer)
                .ReadDependency(csTexture)
                .ReadDependency(lightingAtlasSrv)
                .Done()
            .DeclarePass(RenderGraph::PassType::Compute)
                .SetName("Discard pass")
                .SetExecuteFunction([](RenderGraph::RenderGraph&, RenderGraph::PassExecutionData&){})
                .ReadDependency(lightingAtlasSrv)
                .ReadDependency(csTexture);

        renderGraph.SubmitFrame();
    }

    return 0;
}