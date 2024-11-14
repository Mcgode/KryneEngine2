/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#pragma once

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
        SimplePool<PassDeclaration> m_passDeclarations;
        eastl::vector<SimplePoolHandle> m_declaredPasses;
    };
} // namespace KryneEngine::Modules::RenderGraph
