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
    template<typename T, bool SimdOptimal, bool RowMajor>
    struct Matrix44Base
    {
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

        template <class U, bool S>
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
        [[nodiscard]] Matrix44Base Transposed();

        Vector4Base<T, SimdOptimal> m_vectors[4];
    };
}
