/**
 * @file
 * @author Max Godefroy
 * @date 28/11/2024.
 */

#pragma once

#include <cstddef>
#include <type_traits>

#include "KryneEngine/Core/Math/Vector2.hpp"

namespace KryneEngine::Math
{
    template <typename T, size_t Alignment = sizeof(T)>
    struct alignas(Alignment) Vector3Base
    {
        Vector3Base(): x(), y(), z()
        {
            if constexpr (Alignment > sizeof(T))
            {
                // Ensure padding is zero-initialized
                new (&x + 3) T(0);
            }
        }

        template <typename U0, typename U1 = U0, typename U2 = U0>
        requires std::is_constructible_v<T, U0> && std::is_constructible_v<T, U1> && std::is_constructible_v<T, U2>
        Vector3Base(U0 _x, U1 _y, U2 _z = 0) : x(_x), y(_y), z(_z)
        {
            if constexpr (Alignment > sizeof(T))
            {
                // Ensure padding is zero-initialized
                new (&x + 3) T(0);
            }
        }

        template <typename U>
        requires std::is_constructible_v<T, U>
        explicit Vector3Base(U _value) : Vector3Base(_value, _value, _value) {}

        template <typename U, size_t OtherAlignment>
        requires std::is_constructible_v<T, U>
        explicit Vector3Base(const Vector3Base<U, OtherAlignment> &_other) : Vector3Base(_other.x, _other.y, _other.z) {}

        template <typename U0, typename U1, size_t A>
            requires std::is_constructible_v<T, U0> && std::is_constructible_v<T, U1>
        explicit Vector3Base(const Vector2Base<U0, A>& _vec2, U1 _z = 0): Vector3Base(_vec2.x, _vec2.y, _z) {}

        Vector3Base operator+(const Vector3Base& _other) const;
        Vector3Base operator-(const Vector3Base& _other) const;
        Vector3Base operator*(const Vector3Base& _other) const;
        Vector3Base operator/(const Vector3Base& _other) const;

        bool operator==(const Vector3Base& _other) const;

        void Normalize() requires std::is_floating_point_v<T>;
        Vector3Base Normalized() const requires std::is_floating_point_v<T>;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
        union
        {
            struct
            {
                T x = 0;
                T y = 0;
                T z = 0;
            };
            struct
            {
                T r;
                T g;
                T b;
            };
        };
#pragma clang diagnostic pop
    };

    template<typename T, size_t Alignment>
    extern T Dot(const Vector3Base<T, Alignment>& _a, const Vector3Base<T, Alignment>& _b);

    template<typename T, size_t Alignment>
    extern Vector3Base<T, Alignment> CrossProduct(const Vector3Base<T, Alignment>& _a, const Vector3Base<T, Alignment>& _b);
}