/**
 * @file
 * @author Max Godefroy
 * @date 21/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/Matrix44.hpp"
#include "KryneEngine/Core/Math/Quaternion.hpp"
#include "KryneEngine/Core/Math/RotationConversion.hpp"

namespace KryneEngine::Math
{
    template<Matrix44Type Mat44, Vector3Type Vec3, QuaternionType Quat>
    Mat44 ComputeTransformMatrix(const Vec3& _position, const Quat& _rotation, const Vec3& _scale)
    {
        auto r = ToMatrix33<typename Mat44::ScalarType, Mat44::kSimdOptimal, Mat44::kRowMajorLayout>(_rotation);
        return Mat44 {
            r.Get(0, 0) * _scale.x, r.Get(0, 1) * _scale.y, r.Get(0, 2) * _scale.z, _position.x,
            r.Get(1, 0) * _scale.x, r.Get(1, 1) * _scale.y, r.Get(1, 2) * _scale.z, _position.y,
            r.Get(2, 0) * _scale.x, r.Get(2, 1) * _scale.y, r.Get(2, 2) * _scale.z, _position.z,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }
}