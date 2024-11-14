/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#pragma once

#include <RenderGraph/Declarations/PassAttachmentDeclaration.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    enum class PassType
    {
        Render,
        Compute,
    };

    struct PassDeclaration
    {
    public:
        explicit PassDeclaration(PassType _type);

    public:
        PassType m_type;
        eastl::string m_name;
        eastl::fixed_vector<PassAttachmentDeclaration, RenderPassDesc::kMaxSupportedColorAttachments, false> m_colorAttachments = {};
        eastl::optional<PassAttachmentDeclaration> m_depthAttachment;
    };

    class Builder;

    class PassDeclarationBuilder
    {
        KE_MODULES_RENDER_GRAPH_DECLARATION_BUILDER_IMPL(PassDeclarationBuilder, PassDeclaration, Builder);

    public:
        PassDeclarationBuilder& SetName(const eastl::string_view& _name);
        PassAttachmentDeclarationBuilder AddColorAttachment(SimplePoolHandle _texture);
    };
} // namespace KryneEngine::Module::RenderGraph


