/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include <Profiling/TracyHeader.hpp>
#include <RenderGraph/Declarations/PassDeclaration.hpp>
#include <RenderGraph/Registry.hpp>
#include <RenderGraph/Builder.hpp>

using namespace KryneEngine::Modules;

int main()
{
    TracySetProgramName("Render graph demo");

    RenderGraph::Registry registry {};
    RenderGraph::Builder builder { registry };

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

        swapChainTexture = registry.RegisterRawTexture({}, "Swapchain buffer");
        csTexture = registry.RegisterRawTexture({}, "Compute shader texture");
        texGenBuffer = registry.RegisterRawBuffer({}, "Texture generation buffer");
        frameCBuffer = registry.RegisterRawBuffer({}, "Frame constant buffer");
        lightsBuffer = registry.RegisterRawBuffer({}, "Lights buffer");
        lightingAtlas = registry.RegisterRawTexture({}, "Lighting atlas");
        lightingAtlasSrv = registry.RegisterTextureSrv({}, lightingAtlas, "Lighting atlas SRV");
    }

    {
        KE_ZoneScoped("Build render graph");

        builder
            .DeclareTargetResource(swapChainTexture)
            .DeclarePass(RenderGraph::PassType::Blit)
                .SetName("Upload constant buffer")
                .WriteDependency(frameCBuffer)
                .Done()
            .DeclarePass(RenderGraph::PassType::Compute)
                .SetName("Recompute generative buffer")
                .ReadDependency(texGenBuffer)
                .WriteDependency(texGenBuffer)
                .Done()
            .DeclarePass(RenderGraph::PassType::Compute)
                .SetName("Texture generation")
                .ReadDependency(texGenBuffer)
                .ReadDependency(frameCBuffer)
                .WriteDependency(csTexture)
                .Done()
            .DeclarePass(RenderGraph::PassType::Compute)
                .SetName("Light dispatch")
                .ReadDependency(frameCBuffer)
                .WriteDependency(lightsBuffer)
                .Done()
            .DeclarePass(RenderGraph::PassType::Render)
                .SetName("Light atlas draw")
                .AddColorAttachment(lightingAtlas)
                    .SetLoadOperation(KryneEngine::RenderPassDesc::Attachment::LoadOperation::DontCare)
                    .Done()
                .ReadDependency(lightsBuffer)
                .Done()
            .DeclarePass(RenderGraph::PassType::Render)
                .SetName("Final draw")
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
                .ReadDependency(lightingAtlasSrv)
                .ReadDependency(csTexture);

        builder.PrintBuildResult();
    }

    return 0;
}