/**
 * @file
 * @author Max Godefroy
 * @date 14/11/2024.
 */

#include "KryneEngine/Modules/RenderGraph/Declarations/PassDeclaration.hpp"

namespace KryneEngine::Modules::RenderGraph
{
    PassDeclaration::PassDeclaration(PassType _type, size_t _id)
        : m_type(_type)
        , m_name(eastl::string().sprintf("Pass %zu", _id))
    {}

    PassDeclarationBuilder& PassDeclarationBuilder::SetName(const eastl::string_view& _name)
    {
        m_item.m_name = StringHash(_name);
        return *this;
    }

    PassAttachmentDeclarationBuilder PassDeclarationBuilder::AddColorAttachment(SimplePoolHandle _texture)
    {
        return { m_item.m_colorAttachments.emplace_back(_texture), *this };
    }

    PassAttachmentDeclarationBuilder PassDeclarationBuilder::SetDepthAttachment(SimplePoolHandle _texture)
    {
        m_item.m_depthAttachment.emplace(_texture);
        return { m_item.m_depthAttachment.value(), *this};
    }

    PassDeclarationBuilder& PassDeclarationBuilder::ReadDependency(SimplePoolHandle _resource)
    {
        m_item.m_readDependencies.push_back(_resource);
        return *this;
    }

    PassDeclarationBuilder& PassDeclarationBuilder::WriteDependency(SimplePoolHandle _resource)
    {
        m_item.m_writeDependencies.push_back(_resource);
        return *this;
    }

    PassDeclarationBuilder& PassDeclarationBuilder::SetExecuteFunction(PassDeclaration::ExecuteFunction&& _function)
    {
        m_item.m_executeFunction = _function;
        return *this;
    }
}