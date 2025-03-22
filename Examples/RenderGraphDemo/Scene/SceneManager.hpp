/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#pragma once

#include <KryneEngine/Core/Memory/UniquePtr.hpp>
#include <KryneEngine/Modules/RenderGraph/Declarations/PassDeclaration.hpp>
#include <KryneEngine/Modules/GraphicsUtils/DynamicBuffer.hpp>

namespace KryneEngine::Samples::RenderGraphDemo
{
    class TorusKnot;

    class SceneManager
    {
    public:
        explicit SceneManager(AllocatorInstance _allocator, GraphicsContext* _graphicsContext);
        ~SceneManager();

        void Process(GraphicsContext* _graphicsContext);

        void ExecuteTransfers(GraphicsContext* _graphicsContext, CommandListHandle _commandList);

    private:
        AllocatorInstance m_allocator;
        UniquePtr<TorusKnot> m_torusKnot;

        Modules::GraphicsUtils::DynamicBuffer m_sceneConstantsBuffer;
        DynamicArray<BufferCbvHandle> m_sceneCbvs;
        eastl::vector<u32> m_sceneDescriptorSetIndices;
        DescriptorSetLayoutHandle m_sceneDescriptorSetLayout;
        DynamicArray<DescriptorSetHandle> m_sceneDescriptorSets;
    };
}
