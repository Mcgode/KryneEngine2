/**
 * @file
 * @author Max Godefroy
 * @date 13/03/2025.
 */

#pragma once

#include <cstddef>
#include <EASTL/type_traits.h>

#include "KryneEngine/Core/Math/Vector3.hpp"

namespace KryneEngine::Math
{
    template<class T, size_t Alignment, bool RowMajor>
    class alignas(Alignment) Matrix33Base
    {
        Matrix33Base();
        ~Matrix33Base() = default;

        Matrix33Base(const Matrix33Base<T, Alignment, RowMajor>& _other) = default;
        Matrix33Base(Matrix33Base<T, Alignment, RowMajor>&& _other) = default;

        Matrix33Base<T, Alignment, RowMajor>& operator=(const Matrix33Base<T, Alignment, RowMajor>& _other) = default;
        Matrix33Base<T, Alignment, RowMajor>& operator=(Matrix33Base<T, Alignment, RowMajor>&& _other) = default;

        Matrix33Base(T _a11, T _a12, T _a13, T _a21, T _a22, T _a23, T _a31, T _a32, T _a33);

        template<class U>
        requires eastl::is_convertible_v<U, T>
        Matrix33Base(U _a11, U _a12, U _a13, U _a21, U _a22, U _a23, U _a31, U _a32, U _a33)
            : Matrix33Base(_a11, _a12, _a13, _a21, _a22, _a23, _a31, _a32, _a33) {}

        /**
         * Constructs a 3x3 matrix from three 3D vectors.
         *
         * This constructor initializes the rows or columns of the matrix (depending on
         * the matrix layout, i.e., row-major or column-major) using the input vectors.
         *
         * @tparam U The type of the elements in the input vectors, which must be
         *         convertible to the type of the elements in the matrix (T).
         * @tparam VAlign The alignment requirement of the input vector type.
         * @param _v1 The first 3D vector to populate the first row/column of the matrix.
         * @param _v2 The second 3D vector to populate the second row/column of the matrix.
         * @param _v3 The third 3D vector to populate the third row/column of the matrix.
         *
         * @note Requires that the type `U` is implicitly convertible to the type `T` of
         * the matrix through a constraint using `eastl::is_convertible_v<U, T>`.
         */
        template<class U, size_t VAlign>
        requires eastl::is_convertible_v<U, T>
        Matrix33Base(const Vector3Base<U, VAlign>& _v1, const Vector3Base<U, VAlign>& _v2, const Vector3Base<U, VAlign>& _v3)
            : m_vectors { _v1, _v2, _v3 }
        {}

        template<class U, size_t OtherAlignment>
        requires eastl::is_convertible_v<U, T>
        Matrix33Base(const Matrix33Base<U, OtherAlignment, RowMajor>& _other)
            : m_vectors { _other.m_vectors[0], _other.m_vectors[1], _other.m_vectors[2] }
        {}

        [[nodiscard]] bool IsRowMajor() const { return RowMajor; }
        [[nodiscard]] bool IsColumnMajor() const { return !RowMajor; }

        T& Get(size_t _row, size_t _col);
        const T& Get(size_t _row, size_t _col) const;

        Matrix33Base<T, Alignment, RowMajor> operator+(const Matrix33Base<T, Alignment, RowMajor>& _other) const;
        Matrix33Base<T, Alignment, RowMajor> operator-(const Matrix33Base<T, Alignment, RowMajor>& _other) const;
        Matrix33Base<T, Alignment, RowMajor> operator*(const Matrix33Base<T, Alignment, RowMajor>& _other) const;

        /**
         * Performs an in-place transposition of the 3x3 matrix.
         *
         * This function modifies the matrix by transposing its elements.
         * In a row-major matrix, rows are converted to columns and vice versa, while in a column-major matrix, columns
         * are converted to rows.
         *
         * @return A reference to the transposed matrix.
         */
        Matrix33Base<T, Alignment, RowMajor>& Transpose();

        template <class U, size_t OtherAlignment>
        requires eastl::is_convertible_v<U, T>
        static Matrix33Base<T, Alignment, RowMajor> Convert(const Matrix33Base<U, OtherAlignment, !RowMajor>& _other)
        {
            Matrix33Base<U, OtherAlignment, !RowMajor> transposed = _other;
            transposed.Transpose();
            return Matrix33Base<T, Alignment, RowMajor>(transposed.m_vectors[0], transposed.m_vectors[1], transposed.m_vectors[2]);
        }

        Vector3Base<T, Alignment> m_vectors[3];
    };
}
