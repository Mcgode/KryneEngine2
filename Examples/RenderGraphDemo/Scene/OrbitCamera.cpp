/**
 * @file
 * @author Max Godefroy
 * @date 23/03/2025.
 */

#include "OrbitCamera.hpp"

#include <KryneEngine/Core/Math/CoordinateSystem.hpp>
#include <KryneEngine/Core/Math/Projection.hpp>
#include <KryneEngine/Core/Math/Quaternion.hpp>
#include <KryneEngine/Core/Math/RotationConversion.hpp>
#include <KryneEngine/Core/Math/Transform.hpp>
#include <KryneEngine/Core/Window/Input/InputManager.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

namespace KryneEngine::Samples::RenderGraphDemo
{
    OrbitCamera::OrbitCamera(InputManager* _inputManager, float _aspectRatio)
    : m_aspectRatio(_aspectRatio)
    {
    }

    OrbitCamera::~OrbitCamera() = default;

    void OrbitCamera::Process()
    {
        if (!m_matrixDirty)
        {
            return;
        }

        Math::Quaternion yaw, pitch;
        yaw.FromAxisAngle(Math::UpVector(), m_theta * M_PI / 180.0f);
        pitch.FromAxisAngle(Math::RightVector(), m_phi * M_PI / 180.0f);

        m_viewRotation = yaw * pitch;
        const float3 forwardVector = m_viewRotation.ApplyTo(Math::ForwardVector());
        m_viewTranslation = forwardVector * m_distance - m_focusPosition;

        float4x4_simd viewMatrix = ToMatrix44(ToMatrix33<float3x3_simd>(m_viewRotation));
        Math::SetTranslation(viewMatrix, m_viewTranslation);

        m_projectionViewMatrix = float4x4(
            Math::PerspectiveProjection<float4x4_simd>(
                m_fov,
                m_aspectRatio,
                m_near,
                INFINITY,
                true
            ) * viewMatrix);

        m_depthLinearizeConstants = Math::ComputePerspectiveDepthLinearizationConstants(m_near, INFINITY, true);

        m_matrixDirty = false;
    }
}