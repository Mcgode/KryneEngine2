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

    builder
        .DeclarePass(RenderGraph::PassType::Compute)
            .SetName("Compute pass")
            .WriteDependency(csTexture)
            .Done()
        .DeclarePass(RenderGraph::PassType::Render)
            .SetName("Final draw")
            .AddColorAttachment(swapChainTexture)
                .SetLoadOperation(KryneEngine::RenderPassDesc::Attachment::LoadOperation::Clear)
                .SetStoreOperation(KryneEngine::RenderPassDesc::Attachment::StoreOperation::Store)
                .SetClearColor({ 0, 1, 1, 1 })
                .Done()
            .ReadDependency(csTexture);

    builder.PrintBuildResult();

    return 0;
}