/**
 * @file
 * @author Max Godefroy
 * @date 15/03/2025.
 */

#include "KryneEngine/Core/Math/Matrix44.hpp"

#include "KryneEngine/Core/Math/XSimdUtils.hpp"

namespace KryneEngine::Math
{
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

            for (size_t i = 0; i < Operability::kBatchCount; ++i)
            {
                matB[0 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(_other.m_vectors[0].GetPtr() + Operability::kBatchSize * i);
                matB[1 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(_other.m_vectors[1].GetPtr() + Operability::kBatchSize * i);
                matB[2 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(_other.m_vectors[2].GetPtr() + Operability::kBatchSize * i);
                matB[3 + 4 * i] = XsimdLoad<alignedOps, T, OptimalArch>(_other.m_vectors[3].GetPtr() + Operability::kBatchSize * i);
            }
            for (size_t i = 0; i < Operability::kBatchCount * Operability::kBatchCount; ++i)
            {
                xsimd::transpose(
                    matB + i * Operability::kBatchSize,
                    matB + (i + 1) * Operability::kBatchSize);
            }
            if (Operability::kBatchCount == 2)
            {
                std::swap(matB[2], matB[4]);
                std::swap(matB[3], matB[5]);
            }

            for (auto i = 0; i < 4; ++i)
            {
                for (auto j = 0; j < Operability::kBatchCount; ++j)
                {
                    xsimd::batch matALine = XsimdLoad<alignedOps, T, OptimalArch>(m_vectors[i].GetPtr() + Operability::kBatchSize * j);
                    for (auto k = 0; k < 4; ++k)
                    {
                        if (j == 0)
                        {
                            result.m_vectors[i][k] = xsimd::reduce_add(matALine * matB[k + Operability::kBatchSize * j]);
                        }
                        else
                        {
                            result.m_vectors[i][k] += xsimd::reduce_add(matALine * matB[k + Operability::kBatchSize * j]);
                        }
                    }

                }
            }
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