/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#include "SceneManager.hpp"

#include <KryneEngine/Core/Graphics/Common/ResourceViews/ConstantBufferView.hpp>
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
        , m_sceneCbvs(_allocator)
        , m_sceneDescriptorSetIndices(_allocator)
        , m_sceneDescriptorSets(_allocator)
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
        m_sceneCbvs.Resize(_graphicsContext->GetFrameContextCount());
        for (auto i = 0u; i < _graphicsContext->GetFrameContextCount(); ++i)
        {
            m_sceneCbvs[i] = _graphicsContext->CreateBufferCbv(BufferCbvDesc {
                .m_buffer = m_sceneConstantsBuffer.GetBuffer(i),
                .m_size = sizeof(SceneConstants),
                .m_offset = 0,
            });
        }

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

        m_sceneDescriptorSets.Resize(m_sceneDescriptorSetIndices.size());
        for (auto i = 0; i < m_sceneDescriptorSets.Size(); ++i)
        {
            DescriptorSetHandle& set = m_sceneDescriptorSets[i];
            set = _graphicsContext->CreateDescriptorSet(m_sceneDescriptorSetLayout);

            const DescriptorSetWriteInfo writeInfo {
                .m_index = m_sceneDescriptorSetIndices[0],
                .m_descriptorData = { DescriptorSetWriteInfo::DescriptorData { .m_handle = m_sceneCbvs[i].m_handle } },
            };
            _graphicsContext->UpdateDescriptorSet(set,{ &writeInfo, 1 });
        }
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