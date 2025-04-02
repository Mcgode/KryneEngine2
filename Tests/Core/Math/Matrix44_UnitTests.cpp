/**
 * @file
 * @author Max Godefroy
 * @date 21/03/2025.
 */

#include <gtest/gtest.h>
#include <KryneEngine/Core/Math/Matrix.hpp>
#include <KryneEngine/Core/Math/Projection.hpp>
#include <KryneEngine/Core/Math/Transform.hpp>

namespace KryneEngine::Tests::Math
{
    using namespace KryneEngine::Math;

    TEST(Matrix44, Addition)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        const float4x4 matA {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        const float4x4 matB {
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };

        const float4x4 expectedResult {
            2.0f, 3.0f, 4.0f, 5.0f,
            6.0f, 7.0f, 8.0f, 9.0f,
            10.0f, 11.0f, 12.0f, 13.0f,
            14.0f, 15.0f, 16.0f, 17.0f
        };

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            const float4x4 result = matA + matB;
            EXPECT_EQ(result, expectedResult);
        }

        {
            const float4x4_simd a(matA);
            const float4x4_simd b(matB);
            const float4x4_simd result = a + b;
            EXPECT_EQ(result, float4x4_simd(expectedResult));
        }

        {
            const double4x4 a(matA);
            const double4x4 b(matB);
            const double4x4 result = a + b;
            EXPECT_EQ(result, double4x4(expectedResult));
        }

        {
            const double4x4_simd a(matA);
            const double4x4_simd b(matB);
            const double4x4_simd result = a + b;
            EXPECT_EQ(result, double4x4_simd(expectedResult));
        }
    }

    TEST(Matrix44, Substraction)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        const float4x4 matA {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        const float4x4 matB {
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };

        const float4x4 expectedResult {
            0.0f, 1.0f, 2.0f, 3.0f,
            4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f,
            12.0f, 13.0f, 14.0f, 15.0f
        };

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            const float4x4 result = matA - matB;
            EXPECT_EQ(result, expectedResult);
        }

        {
            const float4x4_simd a(matA);
            const float4x4_simd b(matB);
            const float4x4_simd result = a - b;
            EXPECT_EQ(result, float4x4_simd(expectedResult));
        }

        {
            const double4x4 a(matA);
            const double4x4 b(matB);
            const double4x4 result = a - b;
            EXPECT_EQ(result, double4x4(expectedResult));
        }

        {
            const double4x4_simd a(matA);
            const double4x4_simd b(matB);
            const double4x4_simd result = a - b;
            EXPECT_EQ(result, double4x4_simd(expectedResult));
        }
    }

    TEST(Matrix44, Multiplication)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        const float4x4 matA {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };

        const float4x4 matB {
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };

        const float4x4 expectedResultAb {
            10.f, 10.f, 10.f, 10.f,
            26.f, 26.f, 26.f, 26.f,
            42.f, 42.f, 42.f, 42.f,
            58.f, 58.f, 58.f, 58.f
        };

        const float4x4 expectedResultBa {
            28.f, 32.f, 36.f, 40.f,
            28.f, 32.f, 36.f, 40.f,
            28.f, 32.f, 36.f, 40.f,
            28.f, 32.f, 36.f, 40.f
        };

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            const float4x4 resultAb = matA * matB;
            const float4x4 resultBa = matB * matA;
            EXPECT_EQ(resultAb, expectedResultAb);
            EXPECT_EQ(resultBa, expectedResultBa);
        }

        {
            const float4x4_simd a(matA);
            const float4x4_simd b(matB);
            const float4x4_simd resultAb = a * b;
            const float4x4_simd resultBa = b * a;
            EXPECT_EQ(resultAb, float4x4_simd(expectedResultAb));
            EXPECT_EQ(resultBa, float4x4_simd(expectedResultBa));
        }

        {
            const double4x4 a(matA);
            const double4x4 b(matB);
            const double4x4 resultAb = a * b;
            const double4x4 resultBa = b * a;
            EXPECT_EQ(resultAb, double4x4(expectedResultAb));
            EXPECT_EQ(resultBa, double4x4(expectedResultBa));
        }

        {
            const double4x4_simd a(matA);
            const double4x4_simd b(matB);
            const double4x4_simd resultAb = a * b;
            const double4x4_simd resultBa = b * a;
            EXPECT_EQ(resultAb, double4x4_simd(expectedResultAb));
            EXPECT_EQ(resultBa, double4x4_simd(expectedResultBa));
        }
    }
    
    
    
    TEST(Matrix44, Transpose)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        const float4x4 matBase {
            1.f, 2.f, 3.f, 4.f,
            5.f, 6.f, 7.f, 8.f,
            9.f, 10.f, 11.f, 12.f,
            13.f, 14.f, 15.f, 16.f
        };

        const float4x4 expected {
            1.f, 5.f, 9.f, 13.f,
             2.f, 6.f, 10.f, 14.f,
             3.f, 7.f, 11.f, 15.f,
             4.f, 8.f, 12.f, 16.f
        };

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            EXPECT_EQ(matBase.Transposed(), expected);
        }

        {
            const float4x4_simd mat(matBase);
            EXPECT_EQ(mat.Transposed(), float4x4_simd(expected));
        }

        {
            const double4x4 mat(matBase);
            EXPECT_EQ(mat.Transposed(), double4x4(expected));
        }

        {
            const double4x4_simd mat(matBase);
            EXPECT_EQ(mat.Transposed(), double4x4_simd(expected));
        }
    }

    TEST(Matrix44, InverseFloat4x4)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        const float4x4 identity {};

        const auto translation = ComputeTransformMatrix<float4x4>(
            float3(1.0f, 2.0f, 3.0f),
            Quaternion(),
            float3(1.0f)
        );

        const auto scale = ComputeTransformMatrix<float4x4>(
            float3(),
            Quaternion(),
            float3(1.0f, 0.5f, 1.2f)
        );

        const auto rotation = ComputeTransformMatrix<float4x4>(
            float3(),
            Quaternion(),
            float3(1.0f)
        );

        const auto transform = ComputeTransformMatrix<float4x4>(
            float3(1.0f, 2.0f, 3.0f),
            Quaternion().FromAxisAngle(float3(1.0f, 1.0f, 0.0f).Normalized(), 0.5f),
            float3(1.0f, 0.5f, 1.2f)
        );

        const auto perspective = PerspectiveProjection<float4x4>(1.5f, 1.3333f, 0.1, 1024, false);

        const float4 testVector { 1.0f, 2.0f, 3.0f, 4.0f };

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            const float4 vec = identity * testVector;
            const float4x4 inverse = identity.Inverse();
            EXPECT_EQ(identity, inverse) << "Identity matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Identity matrix inverse is invalid";
        }

        {
            const float4 vec = translation * testVector;
            const float4x4 inverse = translation.Inverse();
            const float4x4 mul = translation * inverse;
            EXPECT_EQ(mul, float4x4()) << "Translation matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Translation matrix inverse is invalid";
        }

        {
            const float4 vec = scale * testVector;
            const float4x4 inverse = scale.Inverse();
            const float4x4 mul = scale * inverse;
            EXPECT_EQ(mul, float4x4()) << "Scale matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Scale matrix inverse is invalid";
        }

        {
            const float4 vec = rotation * testVector;
            const float4x4 inverse = rotation.Inverse();
            const float4x4 mul = rotation * inverse;
            EXPECT_EQ(mul, float4x4()) << "Rotation matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Rotation matrix inverse is invalid";
        }

        {
            const float4 vec = transform * testVector;
            const float4x4 inverse = transform.Inverse();
            const float4x4 mul = transform * inverse;
            EXPECT_EQ(mul, float4x4()) << "Transform matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Transform matrix inverse is invalid";
        }

        {
            const float4 vec = perspective * testVector;
            const float4x4 inverse = perspective.Inverse();
            const float4x4 mul = perspective * inverse;
            EXPECT_EQ(mul, float4x4()) << "Perspective matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Perspective matrix inverse is invalid";
        }
    }

    TEST(Matrix44, InverseDouble4x4)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        const double4x4 identity{};

        const auto translation = double4x4(ComputeTransformMatrix<float4x4>(
            float3(1, 2, 3),
            Quaternion(),
            float3(1.0f)));

        const auto scale = double4x4(ComputeTransformMatrix<float4x4>(
            float3(),
            Quaternion(),
            float3(1.0f, 0.5f, 1.2f)));

        const auto rotation = double4x4(ComputeTransformMatrix<float4x4>(
            float3(),
            Quaternion(),
            float3(1.0f)));

        const auto transform = double4x4(ComputeTransformMatrix<float4x4>(
            float3(1.0f, 2.0f, 3.0f),
            Quaternion().FromAxisAngle(float3(1.0f, 1.0f, 0.0f).Normalized(), 0.5f),
            float3(1.0f, 0.5f, 1.2f)));

        const auto perspective = double4x4(
            PerspectiveProjection<double4x4>(1.5f, 1.3333f, 0.1, 1024, false));

        const double4 testVector{1.0f, 2.0f, 3.0f, 4.0f};

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            const double4 vec = identity * testVector;
            const double4x4 inverse = identity.Inverse();
            EXPECT_EQ(identity, inverse) << "Identity matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Identity matrix inverse is invalid";
        }

        {
            const double4 vec = translation * testVector;
            const double4x4 inverse = translation.Inverse();
            const double4x4 mul = translation * inverse;
            EXPECT_EQ(mul, double4x4()) << "Translation matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Translation matrix inverse is invalid";
        }

        {
            const double4 vec = scale * testVector;
            const double4x4 inverse = scale.Inverse();
            const double4x4 mul = scale * inverse;
            EXPECT_EQ(mul, double4x4()) << "Scale matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Scale matrix inverse is invalid";
        }

        {
            const double4 vec = rotation * testVector;
            const double4x4 inverse = rotation.Inverse();
            const double4x4 mul = rotation * inverse;
            EXPECT_EQ(mul, double4x4()) << "Rotation matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Rotation matrix inverse is invalid";
        }

        {
            const double4 vec = transform * testVector;
            const double4x4 inverse = transform.Inverse();
            const double4x4 mul = transform * inverse;
            EXPECT_EQ(mul, double4x4()) << "Transform matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Transform matrix inverse is invalid";
        }

        {
            const double4 vec = perspective * testVector;
            const double4x4 inverse = perspective.Inverse();
            const double4x4 mul = perspective * inverse;
            EXPECT_EQ(mul, double4x4()) << "Perspective matrix inverse is invalid";
            EXPECT_EQ(testVector, inverse * vec) << "Perspective matrix inverse is invalid";
        }
    }
}