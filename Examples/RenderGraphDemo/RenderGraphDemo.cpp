/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include <RenderGraph/Builder.hpp>
#include <RenderGraph/PassDeclaration.hpp>

using namespace KryneEngine::Modules;

int main()
{
    RenderGraph::Builder builder {};

    builder.DeclarePass(RenderGraph::PassType::Render)
        .SetName("Final draw")
        .AddColorAttachment(
            0,
            KryneEngine::RenderPassDesc::Attachment::LoadOperation::Clear,
            KryneEngine::RenderPassDesc::Attachment::StoreOperation::Store);

    builder.PrintBuildResult();

    return 0;
}