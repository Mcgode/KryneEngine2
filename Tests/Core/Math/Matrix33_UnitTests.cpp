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

    TEST(Matrix33, Addition)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        const float3x3 matA {
            1.0f, 2.0f, 3.0f,
            4.0f, 5.0f, 6.0f,
            7.0f, 8.0f, 9.0f
        };

        const float3x3 matB {
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f
        };

        const float3x3 expectedResult {
            2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f
        };

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            const float3x3 result = matA + matB;
            EXPECT_EQ(result, expectedResult);
        }

        {
            const float3x3_simd a(matA);
            const float3x3_simd b(matB);
            const float3x3_simd result = a + b;
            EXPECT_EQ(result, float3x3_simd(expectedResult));
        }

        {
            const double3x3 a(matA);
            const double3x3 b(matB);
            const double3x3 result = a + b;
            EXPECT_EQ(result, double3x3(expectedResult));
        }

        {
            const double3x3_simd a(matA);
            const double3x3_simd b(matB);
            const double3x3_simd result = a + b;
            EXPECT_EQ(result, double3x3_simd(expectedResult));
        }
    }

    TEST(Matrix33, Substraction)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        const float3x3 matA {
            1.0f, 2.0f, 3.0f,
            4.0f, 5.0f, 6.0f,
            7.0f, 8.0f, 9.0f
        };

        const float3x3 matB {
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f
        };

        const float3x3 expectedResult {
            0.0f, 1.0f, 2.0f,
            3.0f, 4.0f, 5.0f,
            6.0f, 7.0f, 8.0f
        };

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            const float3x3 result = matA - matB;
            EXPECT_EQ(result, expectedResult);
        }

        {
            const float3x3_simd a(matA);
            const float3x3_simd b(matB);
            const float3x3_simd result = a - b;
            EXPECT_EQ(result, float3x3_simd(expectedResult));
        }

        {
            const double3x3 a(matA);
            const double3x3 b(matB);
            const double3x3 result = a - b;
            EXPECT_EQ(result, double3x3(expectedResult));
        }

        {
            const double3x3_simd a(matA);
            const double3x3_simd b(matB);
            const double3x3_simd result = a - b;
            EXPECT_EQ(result, double3x3_simd(expectedResult));
        }
    }

    TEST(Matrix33, Multiplication)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        const float3x3 matA {
            1.0f, 2.0f, 3.0f,
            4.0f, 5.0f, 6.0f,
            7.0f, 8.0f, 9.0f
        };

        const float3x3 matB {
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f
        };

        const float3x3 expectedResultAb {
            6.0f, 6.0f, 6.0f,
            15.0f, 15.0f, 15.0f,
            24.0f, 24.0f, 24.0f
        };

        const float3x3 expectedResultBa {
            12.0f, 15.0f, 18.0f,
            12.0f, 15.0f, 18.0f,
            12.0f, 15.0f, 18.0f
        };

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            const float3x3 resultAb = matA * matB;
            const float3x3 resultBa = matB * matA;
            EXPECT_EQ(resultAb, expectedResultAb);
            EXPECT_EQ(resultBa, expectedResultBa);
        }

        {
            const float3x3_simd a(matA);
            const float3x3_simd b(matB);
            const float3x3_simd resultAb = a * b;
            const float3x3_simd resultBa = b * a;
            EXPECT_EQ(resultAb, float3x3_simd(expectedResultAb));
            EXPECT_EQ(resultBa, float3x3_simd(expectedResultBa));
        }

        {
            const double3x3 a(matA);
            const double3x3 b(matB);
            const double3x3 resultAb = a * b;
            const double3x3 resultBa = b * a;
            EXPECT_EQ(resultAb, double3x3(expectedResultAb));
            EXPECT_EQ(resultBa, double3x3(expectedResultBa));
        }

        {
            const double3x3_simd a(matA);
            const double3x3_simd b(matB);
            const double3x3_simd resultAb = a * b;
            const double3x3_simd resultBa = b * a;
            EXPECT_EQ(resultAb, double3x3_simd(expectedResultAb));
            EXPECT_EQ(resultBa, double3x3_simd(expectedResultBa));
        }
    }
}