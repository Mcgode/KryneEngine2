/**
 * @file
 * @author Max Godefroy
 * @date 23/03/2025.
 */

#pragma once

#include <KryneEngine/Core/Math/Matrix.hpp>
#include <KryneEngine/Core/Window/Input/InputManager.hpp>

namespace KryneEngine::Samples::RenderGraphDemo
{
    class OrbitCamera
    {
    public:
        OrbitCamera(InputManager* _inputManager, float _aspectRatio);
        ~OrbitCamera();

        void Process();

    private:
        float3 m_focusPosition {};
        float m_distance = 10.0f;
        float m_theta = 0.0f;
        float m_phi = 0.0f;
        float m_near = 0.1f;
        float m_fov = 45.0f;
        float m_aspectRatio;

        bool m_matrixDirty = true;

        float4x4 m_projectionViewMatrix {};
    };
}
