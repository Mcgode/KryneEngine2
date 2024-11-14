/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include "Builder.hpp"

#include <iostream>

#include <RenderGraph/PassDeclaration.hpp>
#include <Memory/SimplePool.inl>

namespace KryneEngine::Modules::RenderGraph
{
    Builder::Builder() = default;
    Builder::~Builder() = default;

    PassDeclaration& Builder::DeclarePass(PassType _type)
    {
        SimplePoolHandle declaration = m_passDeclarations.AllocateAndInit(_type);
        m_declaredPasses.push_back(declaration);
        return m_passDeclarations.Get(declaration);
    }

    void Builder::PrintBuildResult()
    {
        std::cout << "Declared passes:" << std::endl;
        std::string indent = "";
        for (SimplePoolHandle passHandle : m_declaredPasses)
        {
            const PassDeclaration& pass = m_passDeclarations.Get(passHandle);

            std::cout << "  - '" << pass.m_name.c_str() << "'" << std::endl;
            indent.push_back('\t');

            if (pass.m_colorAttachments.empty())
            {
                std::cout << indent << "No color attachments" << std::endl;
            }
            else
            {
                std::cout << indent << "Color attachments:" << std::endl;
                for (auto& attachment: pass.m_colorAttachments)
                {
                    std::cout << indent << "  - Texture: " << attachment.m_texture << "; ";
                    switch(attachment.m_loadOperation)
                    {
                    case RenderPassDesc::Attachment::LoadOperation::Load:
                        std::cout << "Load operation: LOAD; ";
                        break;
                    case RenderPassDesc::Attachment::LoadOperation::Clear:
                        std::cout << "Load operation: CLEAR; ";
                        break;
                    case RenderPassDesc::Attachment::LoadOperation::DontCare:
                        std::cout << "Load operation: DONT_CARE; ";
                        break;
                    }
                    switch(attachment.m_storeOperation)
                    {
                    case RenderPassDesc::Attachment::StoreOperation::Store:
                        std::cout << "Store operation: STORE";
                        break;
                    case RenderPassDesc::Attachment::StoreOperation::Resolve:
                        std::cout << "Store operation: RESOLVE";
                        break;
                    case RenderPassDesc::Attachment::StoreOperation::DontCare:
                        std::cout << "Store operation: DONT_CARE";
                        break;
                    }
                    std::cout << std::endl;
                }
            }

            if (pass.m_depthAttachment.has_value())
            {
                std::cout << indent << "Depth/stencil attachment:" << std::endl;
                for (auto& attachment: pass.m_colorAttachments)
                {
                    std::cout << indent << "  - Texture: " << attachment.m_texture << "; ";
                    switch(attachment.m_loadOperation)
                    {
                    case RenderPassDesc::Attachment::LoadOperation::Load:
                        std::cout << "Load operation: LOAD; ";
                        break;
                    case RenderPassDesc::Attachment::LoadOperation::Clear:
                        std::cout << "Load operation: CLEAR; ";
                        break;
                    case RenderPassDesc::Attachment::LoadOperation::DontCare:
                        std::cout << "Load operation: DONT_CARE; ";
                        break;
                    }
                    switch(attachment.m_storeOperation)
                    {
                    case RenderPassDesc::Attachment::StoreOperation::Store:
                        std::cout << "Store operation: STORE;";
                        break;
                    case RenderPassDesc::Attachment::StoreOperation::Resolve:
                        std::cout << "Store operation: RESOLVE;";
                        break;
                    case RenderPassDesc::Attachment::StoreOperation::DontCare:
                        std::cout << "Store operation: DONT_CARE;";
                        break;
                    }
                    std::cout << std::endl;
                }
            }
            else
            {
                std::cout << indent << "No depth/stencil attachment" << std::endl;
            }

            indent.pop_back();
        }
    }
} // namespace KryneEngine::Modules::RenderGraph
