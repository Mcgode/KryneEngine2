/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include <RenderGraph/Declarations/PassDeclaration.hpp>
#include <RenderGraph/Registry.hpp>
#include <RenderGraph/Builder.hpp>

using namespace KryneEngine::Modules;

int main()
{
    RenderGraph::Registry registry {};
    RenderGraph::Builder builder { registry };

    KryneEngine::SimplePoolHandle swapChainTexture = registry.RegisterRawTexture({}, "Swapchain buffer");
    KryneEngine::SimplePoolHandle csTexture = registry.RegisterRawTexture({}, "Compute shader texture");
    KryneEngine::SimplePoolHandle texGenBuffer = registry.RegisterRawBuffer({}, "Texture generation buffer");
    KryneEngine::SimplePoolHandle frameCBuffer = registry.RegisterRawBuffer({}, "Frame constant buffer");
    KryneEngine::SimplePoolHandle lightsBuffer = registry.RegisterRawBuffer({}, "Lights buffer");
    KryneEngine::SimplePoolHandle lightingAtlas = registry.RegisterRawTexture({}, "Lighting atlas");

    builder
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
            .ReadDependency(lightingAtlas);

    builder.PrintBuildResult();

    return 0;
}