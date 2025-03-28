/**
 * @file
 * @author Max Godefroy
 * @date 13/03/2025.
 */

#include "KryneEngine/Core/Math/Matrix33.hpp"

#include "KryneEngine/Core/Math/XSimdUtils.hpp"

namespace KryneEngine::Math
{
    template <class T, bool SimdOptimal, bool RowMajor>
    Matrix33Base<T, SimdOptimal, RowMajor>::Matrix33Base()
        : m_vectors {
              Vector3Base<T, SimdOptimal>( 1, 0, 0 ),
              Vector3Base<T, SimdOptimal>( 0, 1, 0 ),
              Vector3Base<T, SimdOptimal>( 0, 0, 1 ),
          }
    {}

    template <class T, bool SimdOptimal, bool RowMajor>
    Matrix33Base<T, SimdOptimal, RowMajor>::Matrix33Base(T _a11, T _a12, T _a13, T _a21, T _a22, T _a23, T _a31, T _a32, T _a33)
    {
        if constexpr (RowMajor)
        {
            m_vectors[0] = Vector3Base<T, SimdOptimal>(_a11, _a12, _a13);
            m_vectors[1] = Vector3Base<T, SimdOptimal>(_a21, _a22, _a23);
            m_vectors[2] = Vector3Base<T, SimdOptimal>(_a31, _a32, _a33);
        }
        else
        {
            m_vectors[0] = Vector3Base<T, SimdOptimal>(_a11, _a21, _a31);
            m_vectors[1] = Vector3Base<T, SimdOptimal>(_a12, _a22, _a32);
            m_vectors[2] = Vector3Base<T, SimdOptimal>(_a13, _a23, _a33);
        }
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    T& Matrix33Base<T, SimdOptimal, RowMajor>::Get(size_t _row, size_t _col)
    {
        if constexpr (RowMajor)
        {
            return m_vectors[_row][_col];
        }
        else
        {
            return m_vectors[_col][_row];
        }
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    const T& Matrix33Base<T, SimdOptimal, RowMajor>::Get(size_t _row, size_t _col) const
    {
        if constexpr (RowMajor)
        {
            return m_vectors[_row][_col];
        }
        else
        {
            return m_vectors[_col][_row];
        }
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    Matrix33Base<T, SimdOptimal, RowMajor> Matrix33Base<T, SimdOptimal, RowMajor>::operator+(
        const Matrix33Base<T, SimdOptimal, RowMajor>& _other) const
    {
        return Matrix33Base<T, SimdOptimal, RowMajor>(
            m_vectors[0] + _other.m_vectors[0],
            m_vectors[1] + _other.m_vectors[1],
            m_vectors[2] + _other.m_vectors[2]);
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    Matrix33Base<T, SimdOptimal, RowMajor> Matrix33Base<T, SimdOptimal, RowMajor>::operator-(
        const Matrix33Base<T, SimdOptimal, RowMajor>& _other) const
    {
        return Matrix33Base<T, SimdOptimal, RowMajor>(
            m_vectors[0] - _other.m_vectors[0],
            m_vectors[1] - _other.m_vectors[1],
            m_vectors[2] - _other.m_vectors[2]);
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    Matrix33Base<T, SimdOptimal, RowMajor> Matrix33Base<T, SimdOptimal, RowMajor>::operator*(
        const Matrix33Base<T, SimdOptimal, RowMajor>& _other) const
    {
        return Matrix33Base<T, SimdOptimal, RowMajor>(
            Get(0, 0) * _other.Get(0, 0) + Get(0, 1) * _other.Get(1, 0) + Get(0, 2) * _other.Get(2, 0),
            Get(0, 0) * _other.Get(0, 1) + Get(0, 1) * _other.Get(1, 1) + Get(0, 2) * _other.Get(2, 1),
            Get(0, 0) * _other.Get(0, 2) + Get(0, 1) * _other.Get(1, 2) + Get(0, 2) * _other.Get(2, 2),
            Get(1, 0) * _other.Get(0, 0) + Get(1, 1) * _other.Get(1, 0) + Get(1, 2) * _other.Get(2, 0),
            Get(1, 0) * _other.Get(0, 1) + Get(1, 1) * _other.Get(1, 1) + Get(1, 2) * _other.Get(2, 1),
            Get(1, 0) * _other.Get(0, 2) + Get(1, 1) * _other.Get(1, 2) + Get(1, 2) * _other.Get(2, 2),
            Get(2, 0) * _other.Get(0, 0) + Get(2, 1) * _other.Get(1, 0) + Get(2, 2) * _other.Get(2, 0),
            Get(2, 0) * _other.Get(0, 1) + Get(2, 1) * _other.Get(1, 1) + Get(2, 2) * _other.Get(2, 1),
            Get(2, 0) * _other.Get(0, 2) + Get(2, 1) * _other.Get(1, 2) + Get(2, 2) * _other.Get(2, 2)
        );
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    Matrix33Base<T, SimdOptimal, RowMajor>& Matrix33Base<T, SimdOptimal, RowMajor>::Transpose()
    {
        std::swap<T>(m_vectors[0][1], m_vectors[1][0]);
        std::swap<T>(m_vectors[0][2], m_vectors[2][0]);
        std::swap<T>(m_vectors[1][2], m_vectors[2][1]);
        return *this;
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    Matrix33Base<T, SimdOptimal, RowMajor> Matrix33Base<T, SimdOptimal, RowMajor>::Transposed() const
    {
        Matrix33Base transposed = *this;
        transposed.Transpose();
        return transposed;
    }

#define IMPLEMENTATION_INDIVIDUAL(type, simdOptimal, rowMajor) \
    template struct Matrix33Base<type, simdOptimal, rowMajor>

#define IMPLEMENTATION(type)                        \
    IMPLEMENTATION_INDIVIDUAL(type, true, true);    \
    IMPLEMENTATION_INDIVIDUAL(type, true, false);   \
    IMPLEMENTATION_INDIVIDUAL(type, false, false);  \
    IMPLEMENTATION_INDIVIDUAL(type, false, true)

    IMPLEMENTATION(float);
    IMPLEMENTATION(double);

#undef IMPLEMENTATION
#undef IMPLEMENTATION_INDIVIDUAL
}