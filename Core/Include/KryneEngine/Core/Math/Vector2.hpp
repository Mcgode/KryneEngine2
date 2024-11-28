/**
 * @file
 * @author Max Godefroy
 * @date 28/11/2024.
 */

#pragma once

#include <cstddef>
#include <type_traits>

namespace KryneEngine::Math
{
    template <typename T, size_t Alignment = sizeof(T)>
    struct alignas(Alignment) Vector2Base
    {
        Vector2Base() = default;

        template <typename U0, typename U1 = U0>
        requires std::is_constructible_v<T, U0>
            && std::is_constructible_v<T, U1>
        Vector2Base(U0 _x, U1 _y) : x(_x), y(_y) {}

        template <typename U>
        requires std::is_constructible_v<T, U>
        explicit Vector2Base(U _value) : Vector2Base(_value, _value) {}

        template <typename U, size_t OtherAlignment>
        requires std::is_constructible_v<T, U>
        explicit Vector2Base(const Vector2Base<U, OtherAlignment> &_other) : Vector2Base(_other.x, _other.y, _other.z, _other.w)
        {}

        Vector2Base operator+(const Vector2Base& _other) const;
        Vector2Base operator-(const Vector2Base& _other) const;
        Vector2Base operator*(const Vector2Base& _other) const;
        Vector2Base operator/(const Vector2Base& _other) const;

        bool operator==(const Vector2Base& _other) const;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
        union
        {
            struct
            {
                T x = 0;
                T y = 0;
            };
            struct
            {
                T r;
                T g;
            };
        };
#pragma clang diagnostic pop
    };

    template<typename T, size_t Alignment>
    extern T Dot(const Vector2Base<T, Alignment>& _a, const Vector2Base<T, Alignment>& _b);
}