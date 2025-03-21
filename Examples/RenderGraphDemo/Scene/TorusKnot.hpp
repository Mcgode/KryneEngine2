/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/Matrix.hpp"
#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>
#include <KryneEngine/Core/Graphics/Common/Handles.hpp>
#include <KryneEngine/Core/Math/Quaternion.hpp>

namespace KryneEngine::Samples::RenderGraphDemo
{
    class TorusKnot
    {
    public:
        explicit TorusKnot(AllocatorInstance _allocator);

        void Process(GraphicsContext* _graphicsContext);

        void ProcessTransfers(GraphicsContext* _graphicsContext, CommandListHandle _commandList);

        [[nodiscard]] u32 GetRadialSegments() const { return m_radialSegments; }
        void SetRadialSegments(u32 _radialSegments);

        [[nodiscard]] u32 GetTubularSegments() const { return m_tubularSegments; }
        void SetTubularSegments(u32 _tubularSegments);

        [[nodiscard]] float GetKnotRadius() const { return m_knotRadius; }
        void SetKnotRadius(float _knotRadius);

        [[nodiscard]] float GetTubeRadius() const { return m_tubeRadius; }
        void SetTubeRadius(float _tubeRadius);

        [[nodiscard]] u32 GetPValue() const { return m_pValue; }
        void SetPValue(u32 _pValue);

        [[nodiscard]] u32 GetQValue() const { return m_qValue; }
        void SetQValue(u32 _qValue);

    private:
        AllocatorInstance m_allocator;
        bool m_meshDirty = true;

        u32 m_radialSegments = 32;
        u32 m_tubularSegments = 256;
        float m_knotRadius = 1.0f;
        float m_tubeRadius = 0.2f;
        u32 m_pValue = 2;
        u32 m_qValue = 3;

        BufferHandle m_vertexBuffer {};
        BufferHandle m_indexBuffer {};
        BufferHandle m_previousVertexBuffer {};
        BufferHandle m_previousIndexBuffer {};

        BufferHandle m_transferBuffer {};
        u64 m_transferFrameId = 0;
        size_t m_indexBufferSize = 0;
        size_t m_vertexBufferSize = 0;

        float3 m_translation { 0.0f };
        Math::Quaternion m_rotation {};
        float3 m_scale { 1 };
        float4x4 m_modelMatrix {};

        bool m_windowOpen = true;

        void RenderWindow();
    };
}
