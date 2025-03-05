/**
 * @file
 * @author Max Godefroy
 * @date 14/11/2024.
 */

#pragma once

#include <KryneEngine/Core/Graphics/Common/RenderPass.hpp>
#include <KryneEngine/Core/Memory/SimplePool.hpp>

#include "KryneEngine/Modules/RenderGraph/Utils/DeclarationBuilder.hpp"

namespace KryneEngine::Modules::RenderGraph
{
    struct PassAttachmentDeclaration
    {
    public:
        explicit PassAttachmentDeclaration(SimplePoolHandle _texture);

    public:
        SimplePoolHandle m_rtv;
        union
        {
            float4 m_clearColor { 0 };
            struct {
                float m_clearDepth;
                u8 m_clearStencil;
                RenderPassDesc::Attachment::LoadOperation m_stencilLoadOperation;
                RenderPassDesc::Attachment::StoreOperation m_stencilStoreOperation;
            };
        };
        RenderPassDesc::Attachment::LoadOperation m_loadOperation = RenderPassDesc::Attachment::LoadOperation::Load;
        RenderPassDesc::Attachment::StoreOperation m_storeOperation = RenderPassDesc::Attachment::StoreOperation::Store;
        TextureLayout m_layoutBefore = TextureLayout::Unknown;
        TextureLayout m_layoutAfter = TextureLayout::ColorAttachment;
    };

    struct PassDeclarationBuilder;

    class PassAttachmentDeclarationBuilder
    {
        KE_MODULES_RENDER_GRAPH_DECLARATION_BUILDER_IMPL(PassAttachmentDeclarationBuilder, PassAttachmentDeclaration, PassDeclarationBuilder);

    public:
        PassAttachmentDeclarationBuilder& SetLoadOperation(RenderPassDesc::Attachment::LoadOperation _operation);
        PassAttachmentDeclarationBuilder& SetStoreOperation(RenderPassDesc::Attachment::StoreOperation _operation);
        PassAttachmentDeclarationBuilder& SetClearColor(const float4& _clearColor);
        PassAttachmentDeclarationBuilder& SetClearDepthStencil(float _clearDepth, u16 _clearStencil = ~0);
    };

} // namespace KryneEngine::Modules::RenderGraph


