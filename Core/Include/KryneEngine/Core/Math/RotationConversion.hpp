/**
 * @file
 * @author Max Godefroy
 * @date 13/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/CoordinateSystem.hpp"
#include "KryneEngine/Core/Math/Quaternion.hpp"
#include "Vector4.hpp"

namespace KryneEngine::Math
{
    template <class T, class U, EulerOrder Order = kDefaultEulerOrder>
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

    template <class T, class U, size_t Alignment, EulerOrder Order = kDefaultEulerOrder>
        requires std::is_convertible_v<U, T>
    QuaternionBase<T> FromEulerAngles(const Vector3Base<U, Alignment>& _eulerAngles)
    {
        return FromEulerAngles<T, U, Order>(_eulerAngles.x, _eulerAngles.y, _eulerAngles.z);
    }

    template <class T, class U, size_t Alignment = sizeof(T), EulerOrder Order = kDefaultEulerOrder>
        requires std::is_convertible_v<U, T>
    Vector3Base<T, Alignment> ToEulerAngles(const QuaternionBase<U>& _quaternion)
    {
        // Based on: https://stackoverflow.com/a/27496984

        // Aliases for shorter operations
        const U x = _quaternion.x,
                y = _quaternion.y,
                z = _quaternion.z,
                w = _quaternion.w;

        if constexpr (Order == EulerOrder::XYZ)
        {
            return Vector3Base<T, Alignment>(
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
            return Vector3Base<T, Alignment>(
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
            return Vector3Base<T, Alignment>(
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
            return Vector3Base<T, Alignment>(
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
            return Vector3Base<T, Alignment>(
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
            return Vector3Base<T, Alignment>(
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
}