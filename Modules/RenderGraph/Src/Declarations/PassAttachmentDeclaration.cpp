/**
 * @file
 * @author Max Godefroy
 * @date 14/11/2024.
 */

#include "KryneEngine/Modules/RenderGraph/Declarations/PassAttachmentDeclaration.hpp"

namespace KryneEngine::Modules::RenderGraph
{
    PassAttachmentDeclaration::PassAttachmentDeclaration(SimplePoolHandle _texture)
        : m_texture(_texture)
    {}

    PassAttachmentDeclarationBuilder& PassAttachmentDeclarationBuilder::SetLoadOperation(const RenderPassDesc::Attachment::LoadOperation _operation)
    {
        m_item.m_loadOperation = _operation;
        return *this;
    }

    PassAttachmentDeclarationBuilder& PassAttachmentDeclarationBuilder::SetStoreOperation(const RenderPassDesc::Attachment::StoreOperation _operation)
    {
        m_item.m_storeOperation = _operation;
        return *this;
    }

    PassAttachmentDeclarationBuilder& PassAttachmentDeclarationBuilder::SetClearColor(const float4& _clearColor)
    {
        m_item.m_clearColor = _clearColor;
        return *this;
    }

    PassAttachmentDeclarationBuilder& PassAttachmentDeclarationBuilder::SetClearDepthStencil(const float _clearDepth, const u16 _clearStencil)
    {
        m_item.m_clearDepth = _clearDepth;
        m_item.m_clearStencil = _clearStencil;
        return *this;
    }
} // namespace KryneEngine::Modules::RenderGraph

