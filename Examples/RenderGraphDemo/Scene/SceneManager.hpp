/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#pragma once

#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>
#include <KryneEngine/Modules/RenderGraph/Declarations/PassDeclaration.hpp>

namespace KryneEngine::Samples::RenderGraphDemo
{
    class SceneManager
    {
    public:
        explicit SceneManager(AllocatorInstance _allocator);
        ~SceneManager();

        void ExecuteTransfers(Modules::RenderGraph::PassExecutionData& _passExecutionData);

    private:
        AllocatorInstance m_allocator;
    };
}
