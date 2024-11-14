/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include "RenderGraph/Declarations/PassDeclaration.hpp"
#include <RenderGraph/Builder.hpp>

using namespace KryneEngine::Modules;

int main()
{
    RenderGraph::Builder builder {};

    KryneEngine::SimplePoolHandle swapChainTexture = 0;
    KryneEngine::SimplePoolHandle csTexture = 1;
    KryneEngine::SimplePoolHandle texGenBuffer = 2;
    KryneEngine::SimplePoolHandle frameCBuffer = 3;

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
        .DeclarePass(RenderGraph::PassType::Render)
            .SetName("Final draw")
            .AddColorAttachment(swapChainTexture)
                .SetLoadOperation(KryneEngine::RenderPassDesc::Attachment::LoadOperation::Clear)
                .SetStoreOperation(KryneEngine::RenderPassDesc::Attachment::StoreOperation::Store)
                .SetClearColor({ 0, 1, 1, 1 })
                .Done()
            .ReadDependency(frameCBuffer)
            .ReadDependency(csTexture);

    builder.PrintBuildResult();

    return 0;
}