/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#include "TorusKnot.hpp"

#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include <KryneEngine/Core/Graphics/Common/Buffer.hpp>
#include <KryneEngine/Core/Graphics/Common/MemoryBarriers.hpp>
#include <KryneEngine/Core/Math/RotationConversion.hpp>
#include <KryneEngine/Core/Math/Transform.hpp>
#include <imgui.h>

#include "TorusKnotMeshGenerator.hpp"

namespace KryneEngine::Samples::RenderGraphDemo
{
    TorusKnot::TorusKnot(AllocatorInstance _allocator)
        : m_allocator(_allocator)
    {}

    void TorusKnot::Process(GraphicsContext* _graphicsContext)
    {
        if (m_previousIndexBuffer != GenPool::kInvalidHandle && _graphicsContext->IsFrameExecuted(m_transferFrameId - 1))
        {
            _graphicsContext->DestroyBuffer(m_previousIndexBuffer);
            _graphicsContext->DestroyBuffer(m_previousVertexBuffer);
            m_previousIndexBuffer = GenPool::kInvalidHandle;
            m_previousVertexBuffer = GenPool::kInvalidHandle;
        }

        if (m_transferBuffer != GenPool::kInvalidHandle && _graphicsContext->IsFrameExecuted(m_transferFrameId))
        {
            _graphicsContext->DestroyBuffer(m_transferBuffer);
            m_transferBuffer = GenPool::kInvalidHandle;
        }

        if (m_windowOpen)
        {
            RenderWindow();
        }

        if (!m_meshDirty || m_transferBuffer != GenPool::kInvalidHandle)
        {
            return;
        }

        const TorusKnotMeshGenerator::TorusKnotMesh meshData = TorusKnotMeshGenerator::GenerateMesh(
            m_tubularSegments,
            m_radialSegments,
            m_knotRadius,
            m_tubeRadius,
            m_pValue,
            m_qValue,
            m_allocator);

        m_indexBufferSize = meshData.m_indexCount * sizeof(u32);
        m_vertexBufferSize = meshData.m_vertexCount * TorusKnotMeshGenerator::kVertexSize;
        m_transferBuffer = _graphicsContext->CreateBuffer({
            .m_desc = {
                .m_size = m_indexBufferSize + m_vertexBufferSize,
#if !defined(KE_FINAL)
                .m_debugName = "TorusKnotTransferBuffer"
#endif
            },
            .m_usage = MemoryUsage::StageOnce_UsageType | MemoryUsage::TransferSrcBuffer,
        });
        m_transferFrameId = _graphicsContext->GetFrameId();

        m_previousIndexBuffer = m_indexBuffer;
        m_previousVertexBuffer = m_vertexBuffer;

        m_indexBuffer = _graphicsContext->CreateBuffer({
            .m_desc = {
                .m_size = m_indexBufferSize,
#if !defined(KE_FINAL)
                .m_debugName = "TorusKnotIndexBuffer"
#endif
            },
            .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::IndexBuffer | MemoryUsage::TransferDstBuffer,
        });
        m_vertexBuffer = _graphicsContext->CreateBuffer({
            .m_desc = {
                .m_size = m_vertexBufferSize,
#if !defined(KE_FINAL)
                .m_debugName = "TorusKnotVertexBuffer"
#endif
            },
            .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::VertexBuffer | MemoryUsage::TransferDstBuffer,
        });

        BufferMapping mapping(m_transferBuffer, m_indexBufferSize + m_vertexBufferSize);
        _graphicsContext->MapBuffer(mapping);

        memcpy(mapping.m_ptr, meshData.m_indices, m_indexBufferSize);
        memcpy(mapping.m_ptr + m_indexBufferSize, meshData.m_vertices, m_vertexBufferSize);

        m_allocator.Delete(meshData.m_indices);
        m_allocator.Delete(meshData.m_vertices);

        m_meshDirty = false;
    }

    void TorusKnot::ProcessTransfers(GraphicsContext* _graphicsContext, CommandListHandle _commandList)
    {
        if (_graphicsContext->GetFrameId() != m_transferFrameId)
        {
            return;
        }

        KE_ZoneScopedFunction("TorusKnot::ProcessTransfers");

        const BufferMemoryBarrier initBarriers[] = {
            {
                .m_stagesSrc = BarrierSyncStageFlags::None,
                .m_stagesDst = BarrierSyncStageFlags::Transfer,
                .m_accessSrc = BarrierAccessFlags::None,
                .m_accessDst = BarrierAccessFlags::TransferSrc,
                .m_buffer = m_transferBuffer,
            },
            {
                .m_stagesSrc = BarrierSyncStageFlags::None,
                .m_stagesDst = BarrierSyncStageFlags::Transfer,
                .m_accessSrc = BarrierAccessFlags::None,
                .m_accessDst = BarrierAccessFlags::TransferDst,
                .m_buffer = m_indexBuffer,
            },
            {
                .m_stagesSrc = BarrierSyncStageFlags::None,
                .m_stagesDst = BarrierSyncStageFlags::Transfer,
                .m_accessSrc = BarrierAccessFlags::None,
                .m_accessDst = BarrierAccessFlags::TransferDst,
                .m_buffer = m_vertexBuffer,
            },
        };
        _graphicsContext->PlaceMemoryBarriers(_commandList, {}, initBarriers, {});

        _graphicsContext->CopyBuffer(
            _commandList,
            {
                .m_copySize = m_indexBufferSize,
                .m_bufferSrc = m_transferBuffer,
                .m_bufferDst = m_indexBuffer,
                .m_offsetSrc = 0,
            });
        _graphicsContext->CopyBuffer(
            _commandList,
            {
                .m_copySize = m_vertexBufferSize,
                .m_bufferSrc = m_transferBuffer,
                .m_bufferDst = m_vertexBuffer,
                .m_offsetSrc = m_indexBufferSize,
            });

        const BufferMemoryBarrier postCopyBarriers[] = {
            {
                .m_stagesSrc = BarrierSyncStageFlags::Transfer,
                .m_stagesDst = BarrierSyncStageFlags::InputAssembly,
                .m_accessSrc = BarrierAccessFlags::TransferSrc,
                .m_accessDst = BarrierAccessFlags::IndexBuffer,
                .m_buffer = m_indexBuffer,
            },
            {
                .m_stagesSrc = BarrierSyncStageFlags::Transfer,
                .m_stagesDst = BarrierSyncStageFlags::InputAssembly,
                .m_accessSrc = BarrierAccessFlags::TransferSrc,
                .m_accessDst = BarrierAccessFlags::VertexBuffer,
                .m_buffer = m_vertexBuffer,
            }
        };
        _graphicsContext->PlaceMemoryBarriers(_commandList, {}, postCopyBarriers, {});
    }

    void TorusKnot::SetRadialSegments(u32 _radialSegments)
    {
        m_meshDirty = true;
        m_radialSegments = _radialSegments;
    }

    void TorusKnot::SetTubularSegments(u32 _tubularSegments)
    {
        m_meshDirty = true;
        m_tubularSegments = _tubularSegments;
    }

    void TorusKnot::SetKnotRadius(float _knotRadius)
    {
        m_meshDirty = true;
        m_knotRadius = _knotRadius;
    }

    void TorusKnot::SetTubeRadius(float _tubeRadius)
    {
        m_meshDirty = true;
        m_tubeRadius = _tubeRadius;
    }

    void TorusKnot::SetPValue(u32 _pValue)
    {
        m_meshDirty = true;
        m_pValue = _pValue;
    }

    void TorusKnot::SetQValue(u32 _qValue)
    {
        m_meshDirty = true;
        m_qValue = _qValue;
    }

    void TorusKnot::RenderWindow()
    {
        if (!ImGui::Begin("Torus knot", &m_windowOpen))
        {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Transform parameters", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool dirty = false;

            dirty |= ImGui::SliderFloat3("Position", m_translation.GetPtr(), -25, 25);

            auto euler = Math::ToEulerAngles<float3>(m_rotation) * (180. / M_PI);
            if (ImGui::SliderFloat3("Rotation", euler.GetPtr(), -180, 180))
            {
                dirty = true;
                m_rotation = Math::FromEulerAngles<float>(euler * (M_PI / 180.));
            }

            dirty |= ImGui::SliderFloat3("Scale", m_scale.GetPtr(), 0.f, 10.0f);

            if (dirty)
            {
                m_modelMatrix = Math::ComputeTransformMatrix<float4x4>(m_translation, m_rotation, m_scale);
            }
        }

        if (ImGui::CollapsingHeader("Geometry parameters", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const u32 kMinPValue = 1;
            const u32 kMaxPValue = 10;
            m_meshDirty |= ImGui::SliderScalar(
                "P value",
                ImGuiDataType_U32,
                &m_pValue,
                &kMinPValue,
                &kMaxPValue);

            const u32 kMinQValue = 1;
            const u32 kMaxQValue = 10;
            m_meshDirty |= ImGui::SliderScalar(
                "Q value",
                ImGuiDataType_U32,
                &m_qValue,
                &kMinQValue,
                &kMaxQValue);

            const u32 kMinRadialSegments = 3;
            const u32 kMaxRadialSegments = 512;
            m_meshDirty |= ImGui::SliderScalar(
                "Radial segments",
                ImGuiDataType_U32,
                &m_radialSegments,
                &kMinRadialSegments,
                &kMaxRadialSegments);

            const u32 kMinTubularSegments = 16;
            const u32 kMaxTubularSegments = 2048;
            m_meshDirty |= ImGui::SliderScalar(
                "Tubular segments",
                ImGuiDataType_U32,
                &m_tubularSegments,
                &kMinTubularSegments,
                &kMaxTubularSegments);

            m_meshDirty |= ImGui::SliderFloat("Knot radius", &m_knotRadius, 0.0f, 10.0f);
            m_meshDirty |= ImGui::SliderFloat("Tube radius", &m_tubeRadius, 0.0f, 1.0f);
        }

        ImGui::End();
    }
}