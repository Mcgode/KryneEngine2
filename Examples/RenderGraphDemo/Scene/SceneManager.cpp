/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#include "SceneManager.hpp"

#include <KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>

#include "TorusKnot.hpp"

namespace KryneEngine::Samples::RenderGraphDemo
{
    struct SceneConstants
    {
        float4x4 m_torusKnotModel;
        float4x4 m_viewProjection;
    };

    SceneManager::SceneManager(AllocatorInstance _allocator, GraphicsContext* _graphicsContext)
        : m_allocator(_allocator)
        , m_torusKnot(nullptr, _allocator)
        , m_sceneConstantsBuffer(_allocator)
        , m_sceneDescriptorSetIndices(_allocator)
    {
        m_torusKnot.reset(m_allocator.New<TorusKnot>(m_allocator));

        m_sceneConstantsBuffer.Init(
            _graphicsContext,
            {
                .m_desc = {
                    .m_size = sizeof(SceneConstants),
#if !defined(KE_FINAL)
                    .m_debugName = "SceneConstants",
#endif
                },
                .m_usage = MemoryUsage::StageEveryFrame_UsageType | MemoryUsage::ConstantBuffer,
            },
            _graphicsContext->GetFrameContextCount());

        const DescriptorSetDesc sceneDesc = {
            .m_bindings = {
                DescriptorBindingDesc {
                    .m_type = DescriptorBindingDesc::Type::ConstantBuffer,
                    .m_visibility = ShaderVisibility::All
                }
            }
        };

        m_sceneDescriptorSetIndices.resize(sceneDesc.m_bindings.size());
        m_sceneDescriptorSetLayout = _graphicsContext->CreateDescriptorSetLayout(
            sceneDesc,
            m_sceneDescriptorSetIndices.data());
        m_sceneDescriptorSet = _graphicsContext->CreateDescriptorSet(m_sceneDescriptorSetLayout);
    }

    SceneManager::~SceneManager() = default;

    void SceneManager::Process(GraphicsContext* _graphicsContext)
    {
        m_torusKnot->Process(_graphicsContext);
    }

    void SceneManager::ExecuteTransfers(GraphicsContext* _graphicsContext, CommandListHandle _commandList)
    {
        KE_ZoneScopedFunction("SceneManager::ExecuteTransfers");

        m_torusKnot->ProcessTransfers(_graphicsContext, _commandList);
    }
}