/**
 * @file
 * @author Max Godefroy
 * @date 15/03/2025.
 */

#include "KryneEngine/Core/Math/Matrix44.hpp"

#include "KryneEngine/Core/Common/Misc/Unroll.hpp"
#include "KryneEngine/Core/Math/XSimdUtils.hpp"

namespace KryneEngine::Math
{
    template <class T, bool SimdOptimal, bool RowMajor>
    T& Matrix44Base<T, SimdOptimal, RowMajor>::Get(size_t _row, size_t _col)
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
    const T& Matrix44Base<T, SimdOptimal, RowMajor>::Get(size_t _row, size_t _col) const
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

    template <typename T, bool SimdOptimal, bool RowMajor>
    Matrix44Base<T, SimdOptimal, RowMajor> Matrix44Base<T, SimdOptimal, RowMajor>::operator+(const Matrix44Base& _other) const
    {
        return Matrix44Base(
            m_vectors[0] + _other.m_vectors[0],
            m_vectors[1] + _other.m_vectors[1],
            m_vectors[2] + _other.m_vectors[2],
            m_vectors[3] + _other.m_vectors[3]);
    }

    template <typename T, bool SimdOptimal, bool RowMajor>
    Matrix44Base<T, SimdOptimal, RowMajor> Matrix44Base<T, SimdOptimal, RowMajor>::operator-(const Matrix44Base& _other) const
    {
        return Matrix44Base(
            m_vectors[0] - _other.m_vectors[0],
            m_vectors[1] - _other.m_vectors[1],
            m_vectors[2] - _other.m_vectors[2],
            m_vectors[3] - _other.m_vectors[3]);
    }

    template <typename T, bool SimdOptimal, bool RowMajor>
    Matrix44Base<T, SimdOptimal, RowMajor> Matrix44Base<T, SimdOptimal, RowMajor>::operator*(const Matrix44Base& _other) const
    {
        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Matrix44Base>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;
            Matrix44Base result{};

            xsimd::batch<T, OptimalArch> matB[4 * Operability::kBatchCount];

            UNROLL_FOR_LOOP(i, Operability::kBatchCount)
                matB[0 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(_other.m_vectors[0].GetPtr() + Operability::kBatchSize * i);
                matB[1 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(_other.m_vectors[1].GetPtr() + Operability::kBatchSize * i);
                matB[2 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(_other.m_vectors[2].GetPtr() + Operability::kBatchSize * i);
                matB[3 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(_other.m_vectors[3].GetPtr() + Operability::kBatchSize * i);
            END_UNROLL()
            UNROLL_FOR_LOOP(i, Operability::kBatchCount * Operability::kBatchCount)
                xsimd::transpose(
                    matB + i * Operability::kBatchSize,
                    matB + (i + 1) * Operability::kBatchSize);
            END_UNROLL()
            if constexpr (Operability::kBatchCount == 2)
            {
                std::swap(matB[2], matB[4]);
                std::swap(matB[3], matB[5]);
            }

            UNROLL_FOR_LOOP(i, 4)
                UNROLL_FOR_LOOP(j, Operability::kBatchCount)
                    xsimd::batch matALine = XsimdLoad<alignedOps, T, OptimalArch>(
                        m_vectors[i].GetPtr() + Operability::kBatchSize * j);

                    UNROLL_FOR_LOOP(k, 4)
                        if (j == 0)
                        {
                            result.m_vectors[i][k] = xsimd::reduce_add(matALine * matB[k]);
                        }
                        else
                        {
                            result.m_vectors[i][k] += xsimd::reduce_add(matALine * matB[k + 4 * j]);
                        }
                    END_UNROLL()
                END_UNROLL()
            END_UNROLL()

            return result;
        }
        else
        {
            using Vector4 = Vector4Base<T, SimdOptimal>;
            const Vector4& vas = m_vectors;
            const Vector4& vbs = _other.m_vectors;
            return {
                Vector4 {
                    vas[0].x * vbs[0].x + vas[0].y * vbs[1].x + vas[0].z * vbs[2].x + vas[0].w * vbs[3].x,
                    vas[0].x * vbs[0].y + vas[0].y * vbs[1].y + vas[0].z * vbs[2].y + vas[0].w * vbs[3].y,
                    vas[0].x * vbs[0].z + vas[0].y * vbs[1].z + vas[0].z * vbs[2].z + vas[0].w * vbs[3].z,
                    vas[0].x * vbs[0].w + vas[0].y * vbs[1].w + vas[0].z * vbs[2].w + vas[0].w * vbs[3].w
                },
                Vector4 {
                    vas[1].x * vbs[0].x + vas[1].y * vbs[1].x + vas[1].z * vbs[2].x + vas[1].w * vbs[3].x,
                    vas[1].x * vbs[0].y + vas[1].y * vbs[1].y + vas[1].z * vbs[2].y + vas[1].w * vbs[3].y,
                    vas[1].x * vbs[0].z + vas[1].y * vbs[1].z + vas[1].z * vbs[2].z + vas[1].w * vbs[3].z,
                    vas[1].x * vbs[0].w + vas[1].y * vbs[1].w + vas[1].z * vbs[2].w + vas[1].w * vbs[3].w
                },
                Vector4 {
                    vas[2].x * vbs[0].x + vas[2].y * vbs[1].x + vas[2].z * vbs[2].x + vas[2].w * vbs[3].x,
                    vas[2].x * vbs[0].y + vas[2].y * vbs[1].y + vas[2].z * vbs[2].y + vas[2].w * vbs[3].y,
                    vas[2].x * vbs[0].z + vas[2].y * vbs[1].z + vas[2].z * vbs[2].z + vas[2].w * vbs[3].z,
                    vas[2].x * vbs[0].w + vas[2].y * vbs[1].w + vas[2].z * vbs[2].w + vas[2].w * vbs[3].w
                },
                Vector4 {
                    vas[3].x * vbs[0].x + vas[3].y * vbs[1].x + vas[3].z * vbs[2].x + vas[3].w * vbs[3].x,
                    vas[3].x * vbs[0].y + vas[3].y * vbs[1].y + vas[3].z * vbs[2].y + vas[3].w * vbs[3].y,
                    vas[3].x * vbs[0].z + vas[3].y * vbs[1].z + vas[3].z * vbs[2].z + vas[3].w * vbs[3].z,
                    vas[3].x * vbs[0].w + vas[3].y * vbs[1].w + vas[3].z * vbs[2].w + vas[3].w * vbs[3].w
                }
            };
        }
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    Vector4Base<T, SimdOptimal> Matrix44Base<T, SimdOptimal, RowMajor>::operator*(const Vector4Base<T, SimdOptimal>& _other) const
    {
        using Vector4 = Vector4Base<T, SimdOptimal>;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector4>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;

            xsimd::batch<T, OptimalArch> mat[4 * Operability::kBatchCount];
            xsimd::batch<T, OptimalArch> vec[Operability::kBatchCount];

            UNROLL_FOR_LOOP(i, Operability::kBatchCount)
                mat[0 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[0].GetPtr() + Operability::kBatchSize * i);
                mat[1 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[1].GetPtr() + Operability::kBatchSize * i);
                mat[2 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[2].GetPtr() + Operability::kBatchSize * i);
                mat[3 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[3].GetPtr() + Operability::kBatchSize * i);

                vec[i] = XsimdLoad<alignedOps, T, OptimalArch>(_other.GetPtr() + Operability::kBatchSize * i);
            END_UNROLL()

            if constexpr (!RowMajor)
            {
                UNROLL_FOR_LOOP(i, Operability::kBatchCount * Operability::kBatchCount)
                    xsimd::transpose(
                    mat + i * Operability::kBatchSize,
                    mat + (i + 1) * Operability::kBatchSize);
                END_UNROLL()
                if constexpr (Operability::kBatchCount == 2)
                {
                    std::swap(mat[2], mat[4]);
                    std::swap(mat[3], mat[5]);
                }
            }

            Vector4 result {};

            UNROLL_FOR_LOOP(i, Operability::kBatchCount)
                result.x += xsimd::reduce_add(mat[0 + 4 * i] * vec[i]);
                result.y += xsimd::reduce_add(mat[1 + 4 * i] * vec[i]);
                result.z += xsimd::reduce_add(mat[2 + 4 * i] * vec[i]);
                result.w += xsimd::reduce_add(mat[3 + 4 * i] * vec[i]);
            END_UNROLL()

            return result;
        }
        else
        {
            if constexpr (RowMajor)
            {
                return {
                    Dot(m_vectors[0], _other),
                    Dot(m_vectors[1], _other),
                    Dot(m_vectors[2], _other),
                    Dot(m_vectors[3], _other),
                };
            }
            else
            {
                const Matrix44Base transposed = Transposed();
                return {
                    Dot(transposed.m_vectors[0], _other),
                    Dot(transposed.m_vectors[1], _other),
                    Dot(transposed.m_vectors[2], _other),
                    Dot(transposed.m_vectors[3], _other),
                };
            }
        }
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    Matrix44Base<T, SimdOptimal, RowMajor>& Matrix44Base<T, SimdOptimal, RowMajor>::Transpose()
    {
        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Matrix44Base>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;
            xsimd::batch<T, OptimalArch> mat[4 * Operability::kBatchCount];
            UNROLL_FOR_LOOP(i, Operability::kBatchCount)
                mat[0 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[0].GetPtr() + Operability::kBatchSize * i);
                mat[1 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[1].GetPtr() + Operability::kBatchSize * i);
                mat[2 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[2].GetPtr() + Operability::kBatchSize * i);
                mat[3 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[3].GetPtr() + Operability::kBatchSize * i);
            END_UNROLL()
            UNROLL_FOR_LOOP(i, Operability::kBatchCount * Operability::kBatchCount)
                xsimd::transpose(
                    mat + i * Operability::kBatchSize,
                    mat + (i + 1) * Operability::kBatchSize);
            END_UNROLL()
            if constexpr (Operability::kBatchCount == 2)
            {
                std::swap(mat[2], mat[4]);
                std::swap(mat[3], mat[5]);
            }
            UNROLL_FOR_LOOP(i, Operability::kBatchCount)
                XsimdStore<alignedOps, T, OptimalArch>(m_vectors[0].GetPtr() + Operability::kBatchSize * i, mat[0 + 4 * i]);
                XsimdStore<alignedOps, T, OptimalArch>(m_vectors[1].GetPtr() + Operability::kBatchSize * i, mat[1 + 4 * i]);
                XsimdStore<alignedOps, T, OptimalArch>(m_vectors[2].GetPtr() + Operability::kBatchSize * i, mat[2 + 4 * i]);
                XsimdStore<alignedOps, T, OptimalArch>(m_vectors[3].GetPtr() + Operability::kBatchSize * i, mat[3 + 4 * i]);
            END_UNROLL()
        }
        else
        {
            std::swap(m_vectors[0][1], m_vectors[1][0]);
            std::swap(m_vectors[0][2], m_vectors[2][0]);
            std::swap(m_vectors[0][3], m_vectors[3][0]);
            std::swap(m_vectors[1][2], m_vectors[2][1]);
            std::swap(m_vectors[1][3], m_vectors[3][1]);
            std::swap(m_vectors[2][3], m_vectors[3][2]);
        }

        return *this;
    }

    template <typename T, bool SimdOptimal, bool RowMajor>
    Matrix44Base<T, SimdOptimal, RowMajor> Matrix44Base<T, SimdOptimal, RowMajor>::Transposed() const
    {
        Matrix44Base result { *this };
        result.Transpose();
        return result;
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    T Matrix44Base<T, SimdOptimal, RowMajor>::Determinant() const
    {
        // Don't care about the layout, as the determinant of the transpose has the same value
        // https://en.wikipedia.org/wiki/Determinant#Transpose

        const T a0 = m_vectors[0][0],
                a1 = m_vectors[0][1],
                a2 = m_vectors[0][2],
                a3 = m_vectors[0][3],
                b0 = m_vectors[1][0],
                b1 = m_vectors[1][1],
                b2 = m_vectors[1][2],
                b3 = m_vectors[1][3],
                c0 = m_vectors[2][0],
                c1 = m_vectors[2][1],
                c2 = m_vectors[2][2],
                c3 = m_vectors[2][3],
                d0 = m_vectors[3][0],
                d1 = m_vectors[3][1],
                d2 = m_vectors[3][2],
                d3 = m_vectors[3][3];

        return a0*b1*c2*d3 - a0*b1*c3*d2 + a0*b2*c3*d1 - a0*b2*c1*d3 + a0*b3*c1*d2 - a0*b3*c2*d1
            - a1*b2*c3*d0 + a1*b2*c0*d3 - a1*b3*c0*d2 + a1*b3*c2*d0 - a1*b0*c2*d3 + a1*b0*c3*d2
            + a2*b3*c0*d1 - a2*b3*c1*d0 + a2*b0*c1*d3 - a2*b0*c3*d1 + a2*b1*c3*d0 - a2*b1*c0*d3
            - a3*b0*c1*d2 + a3*b0*c2*d1 - a3*b1*c2*d0 + a3*b1*c0*d2 - a3*b2*c0*d1 + a3*b2*c1*d0;
    }

    template <class T, bool SimdOptimal, bool RowMajor>
    Matrix44Base<T, SimdOptimal, RowMajor> Matrix44Base<T, SimdOptimal, RowMajor>::Inverse() const
    {
        // Transpose of an inverse is the inverse of a transpose, so layout is irrelevant.

#define SHUFFLE(a, b,...) xsimd::shuffle(a, b, xsimd::batch_constant<u32, OptimalArch, __VA_ARGS__>())
#define SWIZZLE(v,...) xsimd::swizzle(v, xsimd::batch_constant<u32, OptimalArch, __VA_ARGS__>())

        Matrix44Base result;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector4Base<T, SimdOptimal>>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;

            if constexpr (Operability::kBatchCount == 1)
            {
                using vec4 = xsimd::batch<T, OptimalArch>;

                const vec4 v0 = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[0].GetPtr());
                const vec4 v1 = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[1].GetPtr());
                const vec4 v2 = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[2].GetPtr());
                const vec4 v3 = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[3].GetPtr());

                const vec4 a = SHUFFLE(v0, v1, 0, 1, 4, 5);
                const vec4 b = SHUFFLE(v0, v1, 2, 3, 6, 7);
                const vec4 c = SHUFFLE(v2, v3, 0, 1, 4, 5);
                const vec4 d = SHUFFLE(v2, v3, 2, 3, 6, 7);

                vec4 detA, detB, detC, detD;
                {
                    const vec4 a0 = SHUFFLE(v0, v2, 0, 2, 4, 6);
                    const vec4 b0 = SHUFFLE(v1, v3, 1, 3, 5, 7);
                    const vec4 detMul0 = a0 * b0;

                    const vec4 a1 = SHUFFLE(v0, v2, 1, 3, 5, 7);
                    const vec4 b1 = SHUFFLE(v1, v3, 0, 2, 4, 6);
                    const vec4 detMul1 = a1 * b1;

                    const vec4 detSub = detMul0 - detMul1;

                    detA = SWIZZLE(detSub, 0, 0, 0, 0);
                    detB = SWIZZLE(detSub, 1, 1, 1, 1);
                    detC = SWIZZLE(detSub, 2, 2, 2, 2);
                    detD = SWIZZLE(detSub, 3, 3, 3, 3);
                }

                constexpr auto mat2Mul = [](const vec4& _a, const vec4& _b)
                {
                    return (_a * SWIZZLE(_b, 0, 3, 0, 3))
                           + (SWIZZLE(_a, 1, 0, 3, 2) * SWIZZLE(_b, 2, 1, 2, 1));
                };

                constexpr auto mat2AdjMul = [](const vec4& _a, const vec4& _b)
                {
                    return (SWIZZLE(_a, 3, 3, 0, 0) * _b)
                           - (SWIZZLE(_a, 1, 1, 2, 2) * SWIZZLE(_b, 2, 3, 0, 1));
                };

                constexpr auto mat2MulAdj = [](const vec4& _a, const vec4& _b)
                {
                    return (_a * SWIZZLE(_b, 3, 0, 3, 0))
                           - (SWIZZLE(_a, 1, 0, 3, 2) * SWIZZLE(_b, 2, 1, 2, 1));
                };

                const vec4 d_c = mat2AdjMul(d, c);
                const vec4 a_b = mat2AdjMul(a, b);

                vec4 x_ = (detD * a) - mat2Mul(b, d_c);
                vec4 w_ = (detA * d) - mat2Mul(c, a_b);

                vec4 y_ = (detB * c) - mat2MulAdj(d, a_b);
                vec4 z_ = (detC * b) - mat2MulAdj(a, d_c);

                vec4 detM = (detA * detD) + (detB * detC);

                const T tr = xsimd::reduce_add(a_b * SWIZZLE(d_c, 0, 2, 1, 3));
                detM = detM - tr;

                const vec4 adjSignMask = vec4(1, -1, -1, 1);
                const vec4 invDet = adjSignMask / detM;

                x_ *= invDet;
                y_ *= invDet;
                z_ *= invDet;
                w_ *= invDet;

                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[0].GetPtr(), SHUFFLE(x_, y_, 3, 1, 7, 5));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[1].GetPtr(), SHUFFLE(x_, y_, 2, 0, 6, 4));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[2].GetPtr(), SHUFFLE(z_, w_, 3, 1, 7, 5));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[3].GetPtr(), SHUFFLE(z_, w_, 2, 0, 6, 4));

#undef SWIZZLE
#undef SHUFFLE
            }
            else
            {
#define SHUFFLE(m,...) xsimd::shuffle(m.r0, m.r1, xsimd::batch_constant<u64, OptimalArch, __VA_ARGS__>())
#define SWIZZLE(v,...) xsimd::swizzle(v, xsimd::batch_constant<u64, OptimalArch, __VA_ARGS__>())

                static_assert(Operability::kBatchCount == 2);

                using vec2 = xsimd::batch<T, OptimalArch>;
                struct mat22 {
                    vec2 r0, r1;
                    inline const vec2& operator[](u32 _index) const { return _index == 0 ? r0 : r1; }

                    inline mat22 operator+(const mat22& _o) const { return { r0 + _o.r0, r1 + _o.r1 }; }
                    inline mat22 operator-(const mat22& _o) const { return { r0 - _o.r0, r1 - _o.r1 }; }
                    inline mat22 operator*(const mat22& _o) const { return { r0 * _o.r0, r1 * _o.r1 }; }
                    inline void operator*=(const mat22& _o) { r0 *= _o.r0; r1 *= _o.r1; }
                    inline mat22 operator*(const vec2& _o) const { return { r0 * _o, r1 * _o }; }
                    inline mat22 operator/(const vec2& _o) const { return { r0 / _o, r1 / _o }; }
                };

                const mat22 a = {
                    XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[0].GetPtr()),
                    XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[1].GetPtr()) };
                const mat22 b = {
                    XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[0].GetPtr() + 2),
                    XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[1].GetPtr() + 2) };
                const mat22 c = {
                    XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[2].GetPtr()),
                    XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[3].GetPtr()) };
                const mat22 d = {
                    XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[2].GetPtr() + 2),
                    XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[3].GetPtr() + 2) };

                const vec2 detA = m_vectors[0][0] * m_vectors[1][1] - m_vectors[0][1] * m_vectors[1][0];
                const vec2 detB = m_vectors[0][2] * m_vectors[1][3] - m_vectors[0][3] * m_vectors[1][2];
                const vec2 detC = m_vectors[2][0] * m_vectors[3][1] - m_vectors[2][1] * m_vectors[3][0];
                const vec2 detD = m_vectors[2][2] * m_vectors[3][3] - m_vectors[2][3] * m_vectors[3][2];

                constexpr auto mat2Mul = [](const mat22& _a, const mat22& _b)
                {
                    return mat22 {
                        _a[0] * SHUFFLE(_b, 0, 3) + SWIZZLE(_a[0], 1, 0) * SHUFFLE(_b, 2, 1),
                        _a[1] * SHUFFLE(_b, 0, 3) + SWIZZLE(_a[1], 1, 0) * SHUFFLE(_b, 2, 1),
                    };
                };

                constexpr auto mat2AdjMul = [](const mat22& _a, const mat22& _b)
                {
                    return mat22 {
                        SWIZZLE(_a[1], 1, 1) * _b[0] - SWIZZLE(_a[0], 1, 1) * SWIZZLE(_b[1], 0, 1),
                        SWIZZLE(_a[0], 0, 0) * _b[1] - SWIZZLE(_a[1], 0, 0) * SWIZZLE(_b[0], 0, 1),
                    };
                };

                constexpr auto mat2MulAdj = [](const mat22& _a, const mat22& _b)
                {
                    return mat22 {
                        _a[0] * SHUFFLE(_b, 3, 0) - SWIZZLE(_a[0], 1, 0) * SHUFFLE(_b, 2, 1),
                        _a[1] * SHUFFLE(_b, 3, 0) - SWIZZLE(_a[1], 1, 0) * SHUFFLE(_b, 2, 1),
                    };
                };

                const mat22 d_c = mat2AdjMul(d, c);
                const mat22 a_b = mat2AdjMul(a, b);

                mat22 x_ = a * detD - mat2Mul(b, d_c);
                mat22 y_ = c * detB - mat2MulAdj(d, a_b);
                mat22 z_ = b * detC - mat2MulAdj(a, d_c);
                mat22 w_ = d * detA - mat2Mul(c, a_b);

                vec2 detM = detA * detD - detB * detC;

                const mat22 trMat = a_b * mat22 { SHUFFLE(d_c, 0, 2), SHUFFLE(d_c, 1, 3) };
                const T tr = xsimd::reduce_add(trMat[0] + trMat[1]);
                detM = detM - tr;

                const mat22 adjSignMask { { 1, -1 }, { -1, 1 } };
                const mat22 invDet = adjSignMask / detM;

                x_ *= invDet;
                y_ *= invDet;
                z_ *= invDet;
                w_ *= invDet;

                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[0].GetPtr() + 0, SHUFFLE(x_, 3, 1));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[0].GetPtr() + 2, SHUFFLE(y_, 3, 1));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[1].GetPtr() + 0, SHUFFLE(x_, 2, 0));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[1].GetPtr() + 2, SHUFFLE(y_, 2, 0));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[2].GetPtr() + 0, SHUFFLE(z_, 3, 1));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[2].GetPtr() + 2, SHUFFLE(w_, 3, 1));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[3].GetPtr() + 0, SHUFFLE(z_, 2, 0));
                XsimdStore<alignedOps, T, OptimalArch>(result.m_vectors[3].GetPtr() + 2, SHUFFLE(w_, 2, 0));
#undef SWIZZLE
#undef SHUFFLE
            }
        }
        else
        {

        }

        return result;
    }

#define IMPLEMENTATION_INDIVIDUAL(type, simdOptimal, rowMajor) \
    template struct Matrix44Base<type, simdOptimal, rowMajor>

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