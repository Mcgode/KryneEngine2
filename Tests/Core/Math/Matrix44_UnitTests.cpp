/**
 * @file
 * @author Max Godefroy
 * @date 21/03/2025.
 */

#include <gtest/gtest.h>
#include <KryneEngine/Core/Math/Matrix.hpp>

namespace KryneEngine::Tests::Math
{
    using namespace Math;

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
}