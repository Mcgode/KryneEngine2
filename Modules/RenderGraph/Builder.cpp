/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include "Builder.hpp"

#include <iostream>

#include "RenderGraph/Declarations/PassDeclaration.hpp"
#include <Memory/SimplePool.inl>
#include <RenderGraph/Registry.hpp>
#include <RenderGraph/Resource.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    Builder::Builder(Registry& _registry)
        : m_registry(_registry)
    {}

    Builder::~Builder() = default;

    PassDeclarationBuilder Builder::DeclarePass(PassType _type)
    {
        return { m_declaredPasses.emplace_back(_type), *this };
    }

    void Builder::PrintBuildResult()
    {
        std::cout << "Declared passes:" << std::endl;
        std::string indent = "";
        size_t index = 0;

        m_dag.resize(m_declaredPasses.size());

        for (const PassDeclaration& pass : m_declaredPasses)
        {
            indent.push_back('\t');
            std::cout << "- [" << index <<  "] '" << pass.m_name.c_str() << "' - ";
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
            BuildDag(index, pass);

            indent.pop_back();
            indent.pop_back();

            index++;
        }

        PrintDag();
    }

    void Builder::BuildDag(const size_t _index, const PassDeclaration& _passDeclaration)
    {
        const auto handleResourceRead = [this, _index](SimplePoolHandle _resource)
        {
            const auto versionIt = m_resourceVersions.find(m_registry.GetUnderlyingResource(_resource));
            if (versionIt != m_resourceVersions.end())
            {
                m_dag[versionIt->second.second].m_children.insert(_index);
                m_dag[_index].m_parents.insert(versionIt->second.second);
            }
        };

        const auto handleResourceWrite = [this, _index](SimplePoolHandle _resource)
        {
            m_resourceVersions[_resource].first++;
            m_resourceVersions[_resource].second = _index;
        };

        for (SimplePoolHandle resource: _passDeclaration.m_readDependencies)
        {
            handleResourceRead(resource);
        }
        for (SimplePoolHandle resource: _passDeclaration.m_writeDependencies)
        {
            handleResourceWrite(m_registry.GetUnderlyingResource(resource));
        }

        // Render targets are to be marked as implicit READ/WRITE dependencies
        if (_passDeclaration.m_type == PassType::Render)
        {
            for (const auto& colorAttachment: _passDeclaration.m_colorAttachments)
            {
                handleResourceRead(colorAttachment.m_texture);
                handleResourceWrite(m_registry.GetUnderlyingResource(colorAttachment.m_texture));
            }
            if (_passDeclaration.m_depthAttachment.has_value())
            {
                handleResourceRead(_passDeclaration.m_depthAttachment->m_texture);
                handleResourceWrite(m_registry.GetUnderlyingResource(_passDeclaration.m_depthAttachment->m_texture));
            }
        }
    }

    void Builder::PrintResource(SimplePoolHandle _resource, std::string& _indent)
    {
        const Resource& resource = m_registry.m_resources.Get(_resource);

        std::cout << _indent << "- ";

#if !defined(KE_FINAL)
        if (!resource.m_name.empty())
        {
            std::cout << eastl::string().sprintf("'%s'", resource.m_name.c_str()).c_str();
        }
        else
#endif
        {
            std::cout << "Resource " << _resource;
        }

        switch (resource.m_type)
        {
        case ResourceType::RawTexture:
            std::cout
                << eastl::string().sprintf(", Raw texture, handle: Ox%x",(u32)resource.m_rawTextureData.m_texture.m_handle).c_str()
                << std::endl;
            break;
        case ResourceType::RawBuffer:
            std::cout
                << eastl::string().sprintf(", Raw buffer, handle: Ox%x",(u32)resource.m_rawTextureData.m_texture.m_handle).c_str()
                << std::endl;
            break;
        case ResourceType::Sampler:
            std::cout
                << eastl::string().sprintf(", Sampler, handle: Ox%x",(u32)resource.m_rawTextureData.m_texture.m_handle).c_str()
                << std::endl;
            break;
        case ResourceType::TextureSrv:
            std::cout
                << eastl::string().sprintf(", Texture SRV, handle: Ox%x",(u32)resource.m_rawTextureData.m_texture.m_handle).c_str()
                << std::endl;
            break;
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
                PrintResource(_pass.m_depthAttachment->m_texture, _indent);
                _indent.push_back('\t');
                std::cout << _indent;
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
                _indent.pop_back();
                std::cout << std::endl;
            }
            _indent.pop_back();
        }

        if (_pass.m_depthAttachment.has_value())
        {
            std::cout << _indent << "Depth/stencil attachment:" << std::endl;
            _indent.push_back('\t');
            PrintResource(_pass.m_depthAttachment->m_texture, _indent);
            _indent.push_back('\t');
            std::cout << _indent;
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
                const auto versionIt = m_resourceVersions.find(m_registry.GetUnderlyingResource(resource));
                const u32 version = versionIt != m_resourceVersions.end() ? versionIt->second.first : 0;

                PrintResource(resource, _indent);
                _indent.push_back('\t');
                std::cout << _indent << "Version " << version << std::endl;
                _indent.pop_back();
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
                const u32 version = versionIt != m_resourceVersions.end() ? versionIt->second.first : 0;

                PrintResource(resource, _indent);
                _indent.push_back('\t');
                std::cout << _indent << "Version " << version << std::endl;
                _indent.pop_back();
            }
            _indent.pop_back();
        }
    }

    void Builder::PrintDag()
    {
        std::cout << std::endl;
        std::cout << "DAG:" << std::endl;
        std::cout << "digraph RenderGraph {" << std::endl;

        for (size_t i = 0; i < m_dag.size(); i++)
        {
            const Node& node = m_dag[i];
            for (size_t child: node.m_children)
            {
                std::cout
                    << eastl::string{}.sprintf(R"(  "[%lld] %s" -> "[%lld] %s";)",
                                               i, m_declaredPasses[i].m_name.c_str(),
                                               child, m_declaredPasses[child].m_name.c_str()).c_str()
                    << std::endl;
            }
            if (node.m_children.empty())
            {
                std::cout
                    << eastl::string{}.sprintf(R"(  "[%lld] %s";)", i, m_declaredPasses[i].m_name.c_str()).c_str()
                    << std::endl;
            }
        }
        std::cout << "}" << std::endl;
    }
} // namespace KryneEngine::Modules::RenderGraph
