/**
 * @file
 * @author Max Godefroy
 * @date 13/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/CoordinateSystem.hpp"
#include "KryneEngine/Core/Math/Quaternion.hpp"
#include "KryneEngine/Core/Math/Matrix33.hpp"

namespace KryneEngine::Math
{
    // ============================================================================
    //    DECLARATIONS
    // ============================================================================

    template <class T, class U, EulerOrder Order = kDefaultEulerOrder>
    requires std::is_convertible_v<U, T>
    QuaternionBase<T> FromEulerAngles(U _x, U _y, U _z);

    template <class T, class U, bool SimdOptimal, EulerOrder Order = kDefaultEulerOrder>
        requires std::is_convertible_v<U, T>
    QuaternionBase<T> FromEulerAngles(const Vector3Base<U, SimdOptimal>& _eulerAngles)
    {
        return FromEulerAngles<T, U, Order>(_eulerAngles.x, _eulerAngles.y, _eulerAngles.z);
    }

    template <class T, bool SimdOptimal, bool RowMajor, class U>
        requires std::is_convertible_v<U, T>
    Matrix33Base<T, SimdOptimal, RowMajor> ToMatrix33(const QuaternionBase<U>& _quaternion);

    template <class T, bool VectorSimdOptimal, class U, bool MatrixSimdOptimal, bool RowMajor, EulerOrder Order = kDefaultEulerOrder>
        requires std::is_convertible_v<U, T>
    Vector3Base<T, VectorSimdOptimal> ToEulerAngles(const Matrix33Base<U, MatrixSimdOptimal, RowMajor>& _matrix);

    template <class T, class U, bool SimdOptimal = false, EulerOrder Order = kDefaultEulerOrder>
        requires std::is_convertible_v<U, T>
    Vector3Base<T, SimdOptimal> ToEulerAngles(const QuaternionBase<U>& _quaternion);

    template <class T, bool SimdOptimal, class U>
    requires std::is_convertible_v<U, T>
    std::pair<Vector3Base<T, SimdOptimal>, T> ToAxisAngle(const QuaternionBase<U>& _quaternion);

    // ============================================================================
    //    IMPLEMENTATIONS
    // ============================================================================

    template <class T, class U, EulerOrder Order>
        requires std::is_convertible_v<U, T>
    QuaternionBase<T> FromEulerAngles(U _x, U _y, U _z)
    {
        QuaternionBase<T> result;
        /*
             * Calculated by dividing each angle transform into its own quaternion, which we get using the axis-angle to
             * quaternion transform:
             *  - x: [cos(x/2), sin(x/2), 0, 0]
             *  - y: [cos(y/2), 0, sin(y/2), 0]
             *  - z: [cos(z/2), 0, 0, sin(z/2)]
             *
             *  When working with quaternions, to apply a parent rotation onto a child rotation, we multiply in this
             *  order: Q_parent * Q_child
             *  The multiplication formula can be found here: https://en.wikipedia.org/wiki/Quaternion#Hamilton_product
             *
             *  With each raw order, we multiply like this: Q_first * Q_second * Q_third
             *  The code
         */

        const T cx = std::cos(_x * .5f), sx = std::sin(_x * .5f);
        const T cy = std::cos(_y * .5f), sy = std::sin(_y * .5f);
        const T cz = std::cos(_z * .5f), sz = std::sin(_z * .5f);

        if constexpr (Order == EulerOrder::XYZ)
        {
            // yzW =  cy * cz;
            // yzX =  sy * sz;
            // yzY =  sy * cz;
            // yzZ =  cy * sz;
            result.w = cx * cy * cz - sx * sy * sz;
            result.x = cx * sy * sz + sx * cy * cz;
            result.y = cx * sy * cz - sx * cy * sz;
            result.z = cx * cy * sz + sx * sy * cz;
        }
        else if constexpr (Order == EulerOrder::XZY)
        {
            // yrW =  cr * cy;
            // yrX = -sr * sy;
            // yrY =  sr * cy;
            // yrZ =  cr * sy;
            result.w = cx * cy * cz + sx * sy * sz;
            result.x = sx * cy * cz - cx * sy * sz;
            result.y = cx * sy * cz - sx * cy * sz;
            result.z = cx * cy * sz + sx * sy * cz;
        }
        else if constexpr (Order == EulerOrder::YXZ)
        {
            // pyW =  cp * cy;
            // pyX =  sp * cy;
            // pyY = -sp * sy;
            // pyZ =  cp * sy;
            result.w = cx * cy * cz + sx * sy * sz;
            result.x = sx * cy * cz + cx * sy * sz;
            result.y = cx * sy * cz - sx * cy * sz;
            result.z = cx * cy * sz - sx * sy * cz;
        }
        else if constexpr (Order == EulerOrder::YZX)
        {
            // zxW = cz * cx;
            // zxX = cz * sx;
            // zxY = sz * sx;
            // zxZ = sz * cx;
            result.w = cx * cy * cz - sx * sy * sz;
            result.x = sx * cy * cz + cx * sy * sz;
            result.y = sx * cy * sz + cx * sy * cz;
            result.z = cx * cy * sz - sx * sy * cz;
        }
        else if constexpr (Order == EulerOrder::ZXY)
        {
            // prW = cp * cr;
            // prX = sp * cr;
            // prY = cp * sr;
            // prZ = sp * sr;
            result.w = cx * cy * cz - sx * sy * sz;
            result.x = sx * cy * cz - cx * sy * sz;
            result.y = cx * sy * cz + sx * cy * sz;
            result.z = cx * cy * sz + sx * sy * cz;
        }
        else if constexpr (Order == EulerOrder::ZYX)
        {
            // rpW =  cp * cr;
            // rpX =  sp * cr;
            // rpY =  cp * sr;
            // rpZ = -sp * sr;
            result.w = cx * cy * cz + sx * sy * sz;
            result.x = sx * cy * cz - cx * sy * sz;
            result.y = cx * sy * cz + sx * cy * sz;
            result.z = cx * cy * sz - sx * sy * cz;
        }
        else
        {
            static_assert( false, "Invalid EulerOrder");
        }

        return result;
    }

    template <class T, bool VectorSimdOptimal, class U, bool MatrixSimdOptimal, bool RowMajor, EulerOrder Order>
        requires std::is_convertible_v<U, T>
    Vector3Base<T, VectorSimdOptimal> ToEulerAngles(const Matrix33Base<U, MatrixSimdOptimal, RowMajor>& _matrix)
    {
        // Based on https://github.com/mrdoob/three.js/blob/master/src/math/Euler.js#L188

        Vector3Base<T, VectorSimdOptimal> result;
        constexpr T threshold = 1.0 - QuaternionBase<T>::kQuaternionEpsilon;
        const T& m11 = _matrix.Get(0, 0),
                m12 = _matrix.Get(0, 1),
                m13 = _matrix.Get(0, 2),
                m21 = _matrix.Get(1, 0),
                m22 = _matrix.Get(1, 1),
                m23 = _matrix.Get(1, 2),
                m31 = _matrix.Get(2, 0),
                m32 = _matrix.Get(2, 1),
                m33 = _matrix.Get(2, 2);


        if constexpr (Order == EulerOrder::XYZ)
        {
            result.y = std::asin(m13);
            if (std::abs(m13) < threshold)
            {
                result.x = std::atan2(-m23, m33);
                result.z = std::atan2(-m12, m11);
            }
            else
            {
                result.x = std::atan2(m32, m22);
                result.z = 0;
            }
        }
        else if constexpr (Order == EulerOrder::XZY)
        {
            result.z = std::asin(-m12);
            if (std::abs(m12) < 1.0f - threshold)
            {
                result.x = std::atan2(m32, m22);
                result.y = std::atan2(m13, m11);
            }
            else
            {
                result.x = std::atan2(-m23, m33);
                result.y = 0;
            }
        }
        else if constexpr (Order == EulerOrder::YXZ)
        {
            result.x = std::asin(-m23);
            if (std::abs(m23) < threshold)
            {
                result.y = std::atan2(m13, m33);
                result.z = std::atan2(m21, m22);
            }
            else
            {
                result.y = std::atan2(-m31, m11);
                result.z = 0;
            }
        }
        else if constexpr (Order == EulerOrder::YZX)
        {
            result.z = std::asin(m21);
            if (std::abs(m21) < threshold)
            {
                result.x = std::atan2(-m23, m22);
                result.y = std::atan2(-m31, m11);
            }
            else
            {
                result.x = 0;
                result.y = std::atan2(m13, m33);
            }
        }
        else if constexpr (Order == EulerOrder::ZXY)
        {
            result.x = std::asin(m32);
            if (std::abs(m32) < threshold)
            {
                result.y = std::atan2(-m31, m33);
                result.z = std::atan2(-m12, m22);
            }
            else
            {
                result.y = 0;
                result.x = std::atan2(m21, m11);
            }
        }
        else if constexpr (Order == EulerOrder::ZYX)
        {
            result.y = std::asin(-m31);
            if (std::abs(m31) < threshold)
            {
                result.x = std::atan2(m32, m33);
                result.z = std::atan2(m21, m11);
            }
            else
            {
                result.x = 0;
                result.z = std::atan2(-m12, m22);
            }
        }
        else
        {
            static_assert( false, "Invalid EulerOrder");
        }
        return result;
    }

    template <class T, class U, bool SimdOptimal, EulerOrder Order>
        requires std::is_convertible_v<U, T>
    Vector3Base<T, SimdOptimal> ToEulerAngles(const QuaternionBase<U>& _quaternion)
    {
        // Based on: https://stackoverflow.com/a/27496984

        //        return ToEulerAngles<T, SimdOptimal, U, SimdOptimal, true, Order>(
        //            ToMatrix33<T, SimdOptimal, true>(_quaternion));

        // Aliases for shorter operations
        const U x = _quaternion.x,
                y = _quaternion.y,
                z = _quaternion.z,
                w = _quaternion.w;

        if constexpr (Order == EulerOrder::XYZ)
        {
            return Vector3Base<T, SimdOptimal>(
                std::atan2(
                    -2.0f * (x * y - w * z),
                    w * w + x * x - y * y - z * z),
                std::asin(2.0f * (x * z + w * y)),
                std::atan2(
                    -2.0f * (y * z - w * x),
                    w * w - x * x - y * y + z * z)
            );
        }
        else if constexpr (Order == EulerOrder::XZY)
        {
            return Vector3Base<T, SimdOptimal>(
                std::atan2(
                    2.0f * (x * z + w * y),
                    w * w + x * x - y * y - z * z),
                std::asin(2.0f * (y * z + w * x)),
                std::atan2(
                    -2.0f * (x * y - w * z),
                    w * w - x * x + y * y - z * z)
            );
        }
        else if constexpr (Order == EulerOrder::YXZ)
        {
            return Vector3Base<T, SimdOptimal>(
                std::atan2(
                    2.0f * (x * y + w * z),
                    w * w - x * x + y * y - z * z),
                std::asin(-2.0f * (y * z - w * x)),
                std::atan2(
                    2.0f * (x * z + w * y),
                    w * w - x * x - y * y + z * z)
            );
        }
        else if constexpr (Order == EulerOrder::YZX)
        {
            return Vector3Base<T, SimdOptimal>(
                std::atan2(
                    -2.0f * (y * z - w * x),
                    w * w - x * x + y * y - z * z),
                std::asin(2.0f * (x * y + w * z)),
                std::atan2(
                    -2.0f * (x * z - w * y),
                    w * w + x * x - y * y - z * z)
            );
        }
        else if constexpr (Order == EulerOrder::ZXY)
        {
            return Vector3Base<T, SimdOptimal>(
                std::atan2(
                    -2.0f * (x * z - w * y),
                    w * w - x * x - y * y + z * z),
                std::asin(2.0f * (y * z + w * x)),
                std::atan2(
                    -2.0f * (x * y - w * z),
                    w * w - x * x + y * y - z * z)
            );
        }
        else if constexpr (Order == EulerOrder::ZYX)
        {
            return Vector3Base<T, SimdOptimal>(
                std::atan2(
                    2.0f * (y * z + w * x),
                    w * w - x * x - y * y + z * z),
                std::asin(-2.0f * (x * z - w * y)),
                std::atan2(
                    2.0f * (x * y + w * z),
                    w * w + x * x - y * y - z * z)
            );
        }
        else
        {
            static_assert( false, "Invalid EulerOrder");
        }
    }

    template <class T, bool SimdOptimal, bool RowMajor, class U>
        requires std::is_convertible_v<U, T>
    Matrix33Base<T, SimdOptimal, RowMajor> ToMatrix33(const QuaternionBase<U>& _quaternion)
    {
        const T x(_quaternion.x);
        const T y(_quaternion.y);
        const T z(_quaternion.z);
        const T w(_quaternion.w);

        return Matrix33Base<T, SimdOptimal, RowMajor>(
            1.0f - 2.0f * (y * y + z * z),  2.0f * (x * y - z * w),         2.0f * (x * z + y * w),
            2.0f * (x * y + z * w),         1.0f - 2.0f * (x * x + z * z),  2.0f * (y * z - x * w),
            2.0f * (x * z - y * w),         2.0f * (y * z + x * w),         1.0f - 2.0f * (x * x + y * y));
    }

    template <class T, bool SimdOptimal, class U>
        requires std::is_convertible_v<U, T>
    std::pair<Vector3Base<T, SimdOptimal>, T> ToAxisAngle(const QuaternionBase<U>& _quaternion)
    {
        const T angle = 2.0 * std::acos(_quaternion.w);
        const T s = std::sqrt(1.0 - _quaternion.w * _quaternion.w);
        constexpr T epsilon = 1e-6;

        if (s < epsilon)
        {
            return std::make_pair(Vector3Base<T, SimdOptimal>(1, 0, 0), angle);
        }
        else
        {
            return std::make_pair(Vector3Base<T, SimdOptimal>(_quaternion.x / s, _quaternion.y / s, _quaternion.z / s), angle);
        }
    }
}