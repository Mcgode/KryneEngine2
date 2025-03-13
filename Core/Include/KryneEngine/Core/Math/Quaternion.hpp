/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#pragma once

#include <cmath>
#include <type_traits>

#include "KryneEngine/Core/Math/Vector3.hpp"

namespace KryneEngine::Math
{
    /**
     * @brief Represents a quaternion in 3D space.
     *
     * @details A quaternion is a mathematical representation often used for rotations in 3D space.
     * It consists of a scalar part (w) and a three-dimensional vector part (x, y, z).
     * Quaternions are particularly useful for representing rotations as they avoid the gimbal lock
     * problem inherent in Euler angles and provide more compact and efficient computations than rotation matrices.
     *
     * @tparam T The scalar type used for the quaternion values. Must be a floating-point type.
     */
    template<class T> requires (std::is_floating_point_v<T>)
    struct QuaternionBase
    {
        QuaternionBase()
            : w(1)
            , x(0)
            , y(0)
            , z(0)
        {}

        QuaternionBase(T _w, T _x, T _y, T _z)
            : w(_w)
            , x(_x)
            , y(_y)
            , z(_z)
        {}

        ~QuaternionBase() = default;

        QuaternionBase(const QuaternionBase& _other) = default;
        QuaternionBase(QuaternionBase&& _other) noexcept = default;
        QuaternionBase& operator=(const QuaternionBase& _other) = default;
        QuaternionBase& operator=(QuaternionBase&& _other) noexcept = default;

        template <class U, size_t Alignment>
            requires std::is_convertible_v<U, T>
        void FromAxisAngle(Vector3Base<T, Alignment> _axis, U _angle)
        {
            w = std::cos(_angle * .5f);
            const T s = std::sin(_angle * .5f);
            x =  _axis.x * s;
            y =  _axis.y * s;
            z =  _axis.z * s;
        }



#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
        T w;
        T x;
        T y;
        T z;
#pragma clang diagnostic pop
    };

    using Quaternion = QuaternionBase<float>;
}
