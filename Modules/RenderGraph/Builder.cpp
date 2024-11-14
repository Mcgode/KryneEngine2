/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include "Builder.hpp"

#include <iostream>

#include "RenderGraph/Declarations/PassDeclaration.hpp"
#include <Memory/SimplePool.inl>

namespace KryneEngine::Modules::RenderGraph
{
    Builder::Builder() = default;
    Builder::~Builder() = default;

    PassDeclarationBuilder Builder::DeclarePass(PassType _type)
    {
        SimplePoolHandle declaration = m_passDeclarations.AllocateAndInit(_type);
        m_declaredPasses.push_back(declaration);
        return { m_passDeclarations.Get(declaration), *this };
    }

    void Builder::PrintBuildResult()
    {
        std::cout << "Declared passes:" << std::endl;
        std::string indent = "";
        for (SimplePoolHandle passHandle : m_declaredPasses)
        {
            const PassDeclaration& pass = m_passDeclarations.Get(passHandle);

            indent.push_back('\t');
            std::cout << "- [" << passHandle <<  "] '" << pass.m_name.c_str() << "' - ";
            switch(pass.m_type)
            {
            case PassType::Render:
                std::cout << "RENDER" << std::endl;
                break;
            case PassType::Compute:
                std::cout << "COMPUTE" << std::endl;
                break;
            }
            indent.push_back('\t');

            if (pass.m_type == PassType::Render)
            {
                PrintRenderPassAttachments(pass, indent);
            }
            PrintDependencies(pass, indent);
            for (SimplePoolHandle resource: pass.m_writeDependencies)
            {
                m_resourceVersions[resource]++;
            }

            indent.pop_back();
            indent.pop_back();
        }
    }

    void Builder::PrintRenderPassAttachments(const PassDeclaration& _pass, std::string& _indent)
    {
        if (!_pass.m_colorAttachments.empty())
        {
            std::cout << _indent << "Color attachments:" << std::endl;
            _indent.push_back('\t');
            for (auto& attachment : _pass.m_colorAttachments)
            {
                std::cout << _indent << "  - Texture: " << attachment.m_texture << "; ";
                switch (attachment.m_loadOperation)
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
                switch (attachment.m_storeOperation)
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
            _indent.pop_back();
        }

        if (_pass.m_depthAttachment.has_value())
        {
            std::cout << _indent << "Depth/stencil attachment:" << std::endl;
            _indent.push_back('\t');
            std::cout << _indent << "- Texture: " << _pass.m_depthAttachment.value().m_texture << "; ";
            switch (_pass.m_depthAttachment.value().m_loadOperation)
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
            switch (_pass.m_depthAttachment.value().m_storeOperation)
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
            _indent.pop_back();
        }
    }

    void Builder::PrintDependencies(const PassDeclaration& _pass, std::string& _indent)
    {
        if (!_pass.m_readDependencies.empty())
        {
            std::cout << _indent << "Read dependencies:" << std::endl;
            _indent.push_back('\t');
            for (SimplePoolHandle resource: _pass.m_readDependencies)
            {
                const auto versionIt = m_resourceVersions.find(resource);
                const u32 version = versionIt != m_resourceVersions.end() ? versionIt->second : 0;

                std::cout << _indent << "- Resource " << resource << " version " << version << std::endl;
            }
            _indent.pop_back();
        }

        if (!_pass.m_writeDependencies.empty())
        {
            std::cout << _indent << "Write dependencies:" << std::endl;
            _indent.push_back('\t');
            for (SimplePoolHandle resource: _pass.m_writeDependencies)
            {
                const auto versionIt = m_resourceVersions.find(resource);
                const u32 version = versionIt != m_resourceVersions.end() ? versionIt->second : 0;

                std::cout << _indent << "- Resource " << resource << " version " << version << std::endl;
            }
            _indent.pop_back();
        }
    }
} // namespace KryneEngine::Modules::RenderGraph
