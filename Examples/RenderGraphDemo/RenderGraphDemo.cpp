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

    builder
        .DeclarePass(RenderGraph::PassType::Render)
            .SetName("Final draw")
            .AddColorAttachment(0)
                .SetLoadOperation(KryneEngine::RenderPassDesc::Attachment::LoadOperation::Clear)
                .SetStoreOperation(KryneEngine::RenderPassDesc::Attachment::StoreOperation::DontCare)
                .SetClearColor({ 0, 1, 1, 1 })
                .Done()
            .Done()
        .DeclarePass(RenderGraph::PassType::Compute)
            .SetName("Compute pass");

    builder.PrintBuildResult();

    return 0;
}