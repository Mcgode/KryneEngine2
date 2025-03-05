/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#pragma once

#include <KryneEngine/Core/Common/StringHelpers.hpp>
#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>

#include "KryneEngine/Modules/RenderGraph/Declarations/PassAttachmentDeclaration.hpp"

namespace KryneEngine::Modules::RenderGraph
{
    class RenderGraph;

    enum class PassType
    {
        Render,
        Compute,
        Transfer,
        COUNT,
    };

    struct PassExecutionData
    {
        GraphicsContext* m_graphicsContext;
        CommandListHandle m_commandList;
    };

    struct PassDeclaration
    {
    public:
        explicit PassDeclaration(PassType _type, size_t _id);

        using ExecuteFunction = eastl::function<void(RenderGraph&, PassExecutionData&)>;

        [[nodiscard]] u64 GetRenderPassHash();

    public:
        PassType m_type;
        StringHash m_name;
        eastl::fixed_vector<PassAttachmentDeclaration, RenderPassDesc::kMaxSupportedColorAttachments, false> m_colorAttachments = {};
        eastl::optional<PassAttachmentDeclaration> m_depthAttachment;
        eastl::vector<SimplePoolHandle> m_readDependencies;
        eastl::vector<SimplePoolHandle> m_writeDependencies;
        eastl::optional<u64> m_renderPassHash;
        ExecuteFunction m_executeFunction;
    };

    class Builder;

    class PassDeclarationBuilder
    {
        KE_MODULES_RENDER_GRAPH_DECLARATION_BUILDER_IMPL(PassDeclarationBuilder, PassDeclaration, Builder);

    public:
        PassDeclarationBuilder& SetName(const eastl::string_view& _name);
        PassAttachmentDeclarationBuilder AddColorAttachment(SimplePoolHandle _texture);
        PassAttachmentDeclarationBuilder SetDepthAttachment(SimplePoolHandle _texture);
        PassDeclarationBuilder& ReadDependency(SimplePoolHandle _resource);
        PassDeclarationBuilder& WriteDependency(SimplePoolHandle _resource);
        PassDeclarationBuilder& SetExecuteFunction(PassDeclaration::ExecuteFunction&& _function);
    };
} // namespace KryneEngine::Module::RenderGraph


