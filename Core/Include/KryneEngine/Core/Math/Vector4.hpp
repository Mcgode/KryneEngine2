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
    struct alignas(Alignment) Vector4Base
    {
        Vector4Base() = default;

        template <typename U0, typename U1 = U0, typename U2 = U0, typename U3 = U0>
        requires std::is_constructible_v<T, U0>
            && std::is_constructible_v<T, U1>
            && std::is_constructible_v<T, U2>
            && std::is_constructible_v<T, U3>
        Vector4Base(U0 _x, U1 _y, U2 _z = 0, U3 _w = 0) : x(_x), y(_y), z(_z), w(_w) {}

        template <typename U>
        requires std::is_constructible_v<T, U>
        explicit Vector4Base(U _value) : Vector4Base(_value, _value, _value, _value) {}

        template <typename U, size_t OtherAlignment>
        requires std::is_constructible_v<T, U>
        explicit Vector4Base(const Vector4Base<U, OtherAlignment> &_other) : Vector4Base(_other.x, _other.y, _other.z, _other.w)
        {}

        Vector4Base operator+(const Vector4Base& _other) const;
        Vector4Base operator-(const Vector4Base& _other) const;
        Vector4Base operator*(const Vector4Base& _other) const;
        Vector4Base operator/(const Vector4Base& _other) const;

        bool operator==(const Vector4Base& _other) const;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
        union
        {
            struct
            {
                T x = 0;
                T y = 0;
                T z = 0;
                T w = 0;
            };
            struct
            {
                T r;
                T g;
                T b;
                T a;
            };
        };
#pragma clang diagnostic pop
    };

    template<typename T, size_t Alignment>
    extern T Dot(const Vector4Base<T, Alignment>& _a, const Vector4Base<T, Alignment>& _b);
}