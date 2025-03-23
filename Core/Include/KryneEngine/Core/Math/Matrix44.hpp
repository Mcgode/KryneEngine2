/**
 * @file
 * @author Max Godefroy
 * @date 15/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/Vector4.hpp"
#include "KryneEngine/Core/Math/Matrix33.hpp"

namespace KryneEngine::Math
{
    template<class T, bool SimdOptimal, bool RowMajor>
    struct Matrix44Base
    {
        using ScalarType = T;
        static constexpr bool kSimdOptimal = SimdOptimal;
        static constexpr bool kRowMajorLayout = RowMajor;

        Matrix44Base()
            : m_vectors{
                Vector4Base<T, SimdOptimal>{ 1, 0, 0, 0 },
                Vector4Base<T, SimdOptimal>{ 0, 1, 0, 0 },
                Vector4Base<T, SimdOptimal>{ 0, 0, 1, 0 },
                Vector4Base<T, SimdOptimal>{ 0, 0, 0, 1 }}
        {}

        ~Matrix44Base() = default;

        Matrix44Base(const Matrix44Base&) = default;
        Matrix44Base(Matrix44Base&&) = default;
        Matrix44Base& operator=(const Matrix44Base&) = default;
        Matrix44Base& operator=(Matrix44Base&&) = default;

        template<class U>
        requires std::is_constructible_v<T, U>
        Matrix44Base(
            U _a11, U _a12, U _a13, U _a14,
            U _a21, U _a22, U _a23, U _a24,
            U _a31, U _a32, U _a33, U _a34,
            U _a41, U _a42, U _a43, U _a44)
            : m_vectors {
                  Vector4Base<T, SimdOptimal> { _a11, _a12, _a13, _a14 },
                  Vector4Base<T, SimdOptimal> { _a21, _a22, _a23, _a24 },
                  Vector4Base<T, SimdOptimal> { _a31, _a32, _a33, _a34 },
                  Vector4Base<T, SimdOptimal> { _a41, _a42, _a43, _a44 },
              }
        {}

        template<class U, bool S>
        requires std::is_constructible_v<T, U>
        Matrix44Base(Vector4Base<U, S> _v0, Vector4Base<U, S> _v1, Vector4Base<U, S> _v2, Vector4Base<U, S> _v3)
            : m_vectors{
                  Vector4Base<T, SimdOptimal> { _v0 },
                  Vector4Base<T, SimdOptimal> { _v1 },
                  Vector4Base<T, SimdOptimal> { _v2 },
                  Vector4Base<T, SimdOptimal> { _v3 },
              }
        {}

        template <class U, bool S>
        requires std::is_constructible_v<T, U>
        explicit Matrix44Base(const Matrix44Base<U, S, RowMajor>& _other)
            : Matrix44Base(_other.m_vectors[0], _other.m_vectors[1], _other.m_vectors[2], _other.m_vectors[3])
        {}

        T& Get(size_t _row, size_t _col);
        const T& Get(size_t _row, size_t _col) const;

        Matrix44Base operator+(const Matrix44Base& _other) const;
        Matrix44Base operator-(const Matrix44Base& _other) const;
        Matrix44Base operator*(const Matrix44Base& _other) const;

        bool operator==(const Matrix44Base& _other) const
        {
            return m_vectors[0] == _other.m_vectors[0]
                   && m_vectors[1] == _other.m_vectors[1]
                   && m_vectors[2] == _other.m_vectors[2]
                   && m_vectors[3] == _other.m_vectors[3];
        }

        Matrix44Base& Transpose();
        [[nodiscard]] Matrix44Base Transposed() const;

        Vector4Base<T, SimdOptimal> m_vectors[4];
    };

    template<class T>
    concept Matrix44Type = requires {
        typename T::ScalarType;
        T::kSimdOptimal;
        T::kRowMajorLayout;
        std::is_same_v<Matrix44Base<typename T::ScalarType, T::kSimdOptimal, T::kRowMajorLayout>, T>;
    };
}
