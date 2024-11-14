/**
 * @file
 * @author Max Godefroy
 * @date 14/11/2024.
 */

#include "RenderGraph/Declarations/PassDeclaration.hpp"

namespace KryneEngine::Modules::RenderGraph
{
    PassDeclaration::PassDeclaration(PassType _type)
        : m_type(_type)
    {}

    PassDeclarationBuilder& PassDeclarationBuilder::SetName(const eastl::string_view& _name)
    {
        m_item.m_name = _name;
        return *this;
    }

    PassAttachmentDeclarationBuilder PassDeclarationBuilder::AddColorAttachment(SimplePoolHandle _texture)
    {
        return { m_item.m_colorAttachments.emplace_back(_texture), *this };
    }
}