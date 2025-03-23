/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#include "SceneManager.hpp"

#include <KryneEngine/Core/Graphics/Common/ResourceViews/ConstantBufferView.hpp>
#include <KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include <KryneEngine/Modules/ImGui/Context.hpp>
#include <KryneEngine/Modules/RenderGraph/Builder.hpp>
#include <KryneEngine/Modules/RenderGraph/Registry.hpp>

#include "TorusKnot.hpp"

namespace KryneEngine::Samples::RenderGraphDemo
{
    struct alignas(16) SceneConstants
    {
        float4x4 m_torusKnotModel;
        float4x4 m_viewProjection;
        float3 m_torusKnotAlbedo;
    };

    SceneManager::SceneManager(
        AllocatorInstance _allocator,
        GraphicsContext* _graphicsContext,
        Modules::RenderGraph::Registry& _registry)
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

        m_sceneDescriptorSets.Resize(m_sceneCbvs.Size());
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

        m_cbRenderGraphHandles.Resize(_graphicsContext->GetFrameContextCount());
        m_cbvRenderGraphHandles.Resize(_graphicsContext->GetFrameContextCount());
        for (auto i = 0u; i < m_cbRenderGraphHandles.Size(); ++i)
        {
            m_cbRenderGraphHandles[i] = _registry.RegisterRawBuffer(m_sceneConstantsBuffer.GetBuffer(i));
            m_cbvRenderGraphHandles[i] = _registry.RegisterCbv(m_sceneCbvs[i], m_cbRenderGraphHandles[i]);
        }
    }

    SceneManager::~SceneManager() = default;

    void SceneManager::PreparePsos(GraphicsContext* _graphicsContext, RenderPassHandle _dummyGBufferRenderPass)
    {
        m_torusKnot->BuildPso(
            _graphicsContext,
            _dummyGBufferRenderPass,
            m_sceneDescriptorSetLayout);
    }

    void SceneManager::DeclareDataTransferPass(
        const GraphicsContext* _graphicsContext,
        Modules::RenderGraph::Builder& _builder,
        Modules::ImGui::Context* _imGuiContext)
    {
        const u8 index = _graphicsContext->GetCurrentFrameContextIndex();;

        m_currentCbv = m_cbvRenderGraphHandles[index];

        const auto transferExecuteFunction =
            [this, _imGuiContext](Modules::RenderGraph::RenderGraph& _renderGraph, Modules::RenderGraph::PassExecutionData _passData)
        {
            ExecuteTransfers(_passData.m_graphicsContext, _passData.m_commandList);
            _imGuiContext->PrepareToRenderFrame(_passData.m_graphicsContext, _passData.m_commandList);
        };

        _builder
            .DeclarePass(Modules::RenderGraph::PassType::Transfer)
            .SetName("Scene data transfer pass")
            .SetExecuteFunction(transferExecuteFunction)
            .WriteDependency({
                .m_resource = m_cbRenderGraphHandles[index],
                .m_finalSyncStage { BarrierSyncStageFlags::All },
                .m_finalAccessFlags { BarrierAccessFlags::ConstantBuffer }
            });
    }

    void SceneManager::Process(GraphicsContext* _graphicsContext)
    {
        m_torusKnot->Process(_graphicsContext);

        auto* sceneConstants = static_cast<SceneConstants*>(m_sceneConstantsBuffer.Map(
            _graphicsContext,
            _graphicsContext->GetCurrentFrameContextIndex()));

        sceneConstants->m_torusKnotModel = m_torusKnot->GetModelMatrix();
        sceneConstants->m_viewProjection = float4x4();
        sceneConstants->m_torusKnotAlbedo = m_torusKnot->GetAlbedo();

        m_sceneConstantsBuffer.Unmap(_graphicsContext);
    }

    void SceneManager::ExecuteTransfers(GraphicsContext* _graphicsContext, CommandListHandle _commandList)
    {
        KE_ZoneScopedFunction("SceneManager::ExecuteTransfers");

        m_sceneConstantsBuffer.PrepareBuffers(
            _graphicsContext,
            _commandList,
            BarrierAccessFlags::ConstantBuffer,
            _graphicsContext->GetCurrentFrameContextIndex());

        m_torusKnot->ProcessTransfers(_graphicsContext, _commandList);
    }

    void SceneManager::RenderGBuffer(GraphicsContext* _graphicsContext, CommandListHandle _commandList)
    {
        _graphicsContext->DeclarePassBufferCbvUsage(_commandList, { &m_sceneCbvs[_graphicsContext->GetCurrentFrameContextIndex()], 1 });
        m_torusKnot->RenderGBuffer(_graphicsContext, _commandList, m_sceneDescriptorSets[_graphicsContext->GetCurrentFrameContextIndex()]);
    }
}