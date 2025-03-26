/**
 * @file
 * @author Max Godefroy
 * @date 28/02/2025.
 */

#include "TorusKnotMeshGenerator.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <KryneEngine/Core/Math/Vector3.hpp>

namespace
{
    using namespace KryneEngine;

    float3_simd computePositionOnCurve(float _u, float _p, float _q, float _knotRadius)
    {
        const float cu = std::cos(_u);
        const float su = std::sin(_u);
        const float quOverP = _q * _u / _p;
        const float cs = std::cos(quOverP);

        return float3_simd {
            _knotRadius * (2 + cs) * cu * 0.5f,
            _knotRadius * (2 + cs) * su * 0.5f,
            _knotRadius * std::sin(quOverP) * _q * 0.5f
        };
    }
}

namespace KryneEngine::Samples::RenderGraphDemo::TorusKnotMeshGenerator
{
    TorusKnotMesh GenerateMesh(
        u32 _tubularSegments,
        u32 _radialSegments,
        float _knotRadius,
        float _tubeRadius,
        u32 _p,
        u32 _q,
        AllocatorInstance _allocator)
    {
        TorusKnotMesh mesh {};

        mesh.m_indexCount = 3 * 2 * _tubularSegments * _radialSegments;
        mesh.m_vertexCount = (_tubularSegments + 1) * (_radialSegments + 1);

        mesh.m_indices = static_cast<std::byte*>(_allocator.allocate(mesh.m_indexCount * sizeof(u32)));
        mesh.m_vertices = static_cast<std::byte*>(_allocator.allocate(mesh.m_vertexCount * sizeof(float) * 3));

        const auto p = static_cast<float>(_p);
        const auto q = static_cast<float>(_q);

        u32 vertexId = 0;
        for (auto i = 0u; i <= _tubularSegments; i++)
        {
            const float u = static_cast<float>(i) / static_cast<float>(_tubularSegments) * float(M_PI) * 2.f;

            const float3_simd p1 = computePositionOnCurve(u, p, q, _knotRadius);
            const float3_simd p2 = computePositionOnCurve(u + 0.01f, p, q, _knotRadius);

            float3_simd t = p2 - p1;
            float3_simd n = p1 + p2;
            float3_simd b = float3_simd::CrossProduct(t, n);
            n = float3_simd::CrossProduct(b, t);

            n.Normalize();
            b.Normalize();

            for (auto j = 0u; j <= _radialSegments; j++)
            {
                const float v = static_cast<float>(j) / static_cast<float>(_radialSegments) * float(M_PI) * 2.f;
                const float3_simd cx(-_tubeRadius * std::cos(v));
                const float3_simd cy(_tubeRadius * std::sin(v));

                const float3_simd position = p1 + (cx * n) + (cy * b);
                memcpy(
                    mesh.m_vertices + vertexId * kVertexSize + kVertexPositionOffset,
                    position.GetPtr(),
                    kVertexPositionSize);

                const float3_simd normal = (position - p1).Normalized();
                memcpy(
                    mesh.m_vertices + vertexId * kVertexSize + kVertexNormalOffset,
                    normal.GetPtr(),
                    kVertexNormalSize);

                vertexId++;
            }
        }

        u32* indexPtr = reinterpret_cast<u32*>(mesh.m_indices);
        for (auto i = 0u; i < _tubularSegments; i++)
        {
            for (auto j = 0u; j < _radialSegments; j++)
            {
                const u32 a = (_radialSegments + 1) * j + i;
                const u32 b = (_radialSegments + 1) * (j + 1) + i;
                const u32 c = (_radialSegments + 1) * (j + 1) + i + 1;
                const u32 d = (_radialSegments + 1) * j + i + 1;

                *(indexPtr++) = a;
                *(indexPtr++) = b;
                *(indexPtr++) = d;

                *(indexPtr++) = b;
                *(indexPtr++) = c;
                *(indexPtr++) = d;
            }
        }

        return mesh;
    }
}

