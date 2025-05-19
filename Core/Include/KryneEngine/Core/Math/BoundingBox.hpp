/**
 * @file
 * @author Max Godefroy
 * @date 19/05/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/Vector.hpp"

namespace KryneEngine::Math
{
    struct BoundingBox
    {
        float3 m_min { +FLT_MAX };
        float3 m_max { -FLT_MAX };

        BoundingBox() = default;
        BoundingBox(const float3& _min, const float3& _max): m_min(_min), m_max(_max) {}
        explicit BoundingBox(const float3& _point): m_min(_point), m_max(_point) {}

        [[nodiscard]] float3 GetCenter() const
        {
            return (m_min + m_max) * 0.5f;
        }

        [[nodiscard]] float3 GetSize() const
        {
            return m_max - m_min;
        }

        void Expand(const float3& _point)
        {
            m_min.MinComponents(_point);
            m_max.MaxComponents(_point);
        }
    };
}