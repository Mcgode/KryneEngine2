/**
 * @file
 * @author Max Godefroy
 * @date 19/06/2025.
 */

#include <gtest/gtest.h>
#include <KryneEngine/Core/Math/Float16.hpp>

namespace KryneEngine::Tests::Math
{
    using namespace KryneEngine::Math;

    void TestF32ToF16(u32 _float32, u16 _float16)
    {
        const float value = *reinterpret_cast<const float*>(&_float32);

        const u16 computed = Float16::ConvertToFloat16(value);
        EXPECT_EQ(computed, _float16);
    }

    TEST(Float16, F32ToF16)
    {
        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        // See https://evanw.github.io/float-toy/

        // 3.1415927 -> 3.141
        TestF32ToF16(0b0'10000000'10010010000111111011011, 0b0'10000'1001001000);

        // -0.5 -> -0.5
        TestF32ToF16(0b1'01111110'00000000000000000000000, 0b1'01110'0000000000);

        // 1.23456 -> 1.235
        TestF32ToF16(0b0'01111111'00111100000011000010000, 0b0'01111'0011110000);

        // -Inf -> -Inf
        TestF32ToF16(0b1'11111111'00000000000000000000000, 0b1'11111'0000000000);

        // +Inf -> +Inf
        TestF32ToF16(0b0'11111111'00000000000000000000000, 0b0'11111'0000000000);

        // Nan -> Nan
        TestF32ToF16(0b1'11111111'11111111111111111111111, 0b1'11111'1111111111);

        // 1e-8 -> 0
        TestF32ToF16(0b0'01100100'01010111100110001110111, 0b0'00000'0000000000);

        // -1e8 -> -Inf
        TestF32ToF16(0b1'10011001'01111101011110000100000, 0b1'11111'0000000000);
    }

    void TestF16ToF32(u16 _float16, u32 _float32)
    {
        const float computed = Float16::ConvertFromFloat16(_float16);
        const uint32_t computedU = *reinterpret_cast<const uint32_t*>(&computed);
        EXPECT_EQ(computedU, _float32);
    }

    TEST(Float16, F16ToF32)
    {
        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        // See https://evanw.github.io/float-toy/

        // 3.141 -> 3.141
        TestF16ToF32(0b0'10000'1001001000, 0b0'10000000'10010010000000000000000);

        // -0.5 -> -0.5
        TestF16ToF32(0b1'01110'0000000000, 0b1'01111110'00000000000000000000000);

        // 1.235 -> 1.234375
        TestF16ToF32(0b0'01111'0011110000, 0b0'01111111'00111100000000000000000);

        // -Inf -> -Inf
        TestF16ToF32(0b1'11111'0000000000, 0b1'11111111'00000000000000000000000);

        // +Inf -> +Inf
        TestF16ToF32(0b0'11111'0000000000, 0b0'11111111'00000000000000000000000);

        // Nan -> Nan
        TestF16ToF32(0b1'11111'1111111111, 0b1'11111111'11111111110000000000000);
    }
}