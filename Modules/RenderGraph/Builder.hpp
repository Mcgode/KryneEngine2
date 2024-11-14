/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#pragma once

#include <EASTL/hash_map.h>
#include <Memory/SimplePool.hpp>
#include <RenderGraph/Declarations/PassDeclaration.hpp>

namespace KryneEngine
{
    struct RenderPassDesc;
}

namespace KryneEngine::Modules::RenderGraph
{


    enum class PassType;

    class Builder
    {
    public:
        Builder();
        ~Builder();

    public:
        PassDeclarationBuilder DeclarePass(PassType _type);

        void PrintBuildResult();

    private:
        static void PrintRenderPassAttachments(const PassDeclaration& _pass, std::string& _indent);
        void PrintDependencies(const PassDeclaration& _pass, std::string& _indent);

    private:
        SimplePool<PassDeclaration> m_passDeclarations;
        eastl::vector<SimplePoolHandle> m_declaredPasses;
        eastl::hash_map<SimplePoolHandle, u32> m_resourceVersions;
    };
} // namespace KryneEngine::Modules::RenderGraph
