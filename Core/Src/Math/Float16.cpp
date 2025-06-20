/**
 * @file
 * @author Max Godefroy
 * @date 19/06/2025.
 */

#include "KryneEngine/Core/Math/Float16.hpp"

#if defined(__ARM_NEON) && defined(__ARM_FP16_FORMAT_IEEE)
#   include <arm_neon.h>
#   define KE_ARM_NEON
#endif

namespace KryneEngine::Math
{
    Float16::Float16(float _value)
        : m_data(ConvertToFloat16(_value))
    {}

    Float16& Float16::operator=(float _value)
    {
        m_data = ConvertToFloat16(_value);
        return *this;
    }

    Float16::operator float() const
    {
        return ConvertFromFloat16(m_data);
    }

    u16 Float16::ConvertToFloat16(float _value)
    {
#if defined(KE_ARM_NEON)
        float32x4_t val { _value };
        float16x4_t result = vcvt_f16_f32(val);
        return *reinterpret_cast<u16*>(&result);
#else
        const u32 binaryFloat = *reinterpret_cast<u32*>(&_value);

        const u32 sign = (binaryFloat >> 31) & 0x1;
        const u32 exponent = (binaryFloat >> 23) & 0xff;
        const u32 mantissa = binaryFloat & 0x7fffff;

        // Zero
        if (exponent == 0)
        {
            return sign << 15;
        }
        // Handle Nan and Infinity
        else if (exponent == 0xff)
        {
            if (mantissa == 0)
                // Infinity
                return (sign << 15) | (0x1f << 10);
            else
                // NaN
                return (sign << 15) | (0x1f << 10) | 0x3FF;
        }

        const s32 exponent16 = static_cast<s32>(exponent) - 127 + 15;

        // Handle overflow and underflow
        if (exponent16 >= 31)
        {
            // Overflow to infinity
            return (sign << 15) | (0x1f << 10);
        }
        else if (exponent16 <= 0)
        {
            // Underflow or subnormal
            if (exponent16 >= -10)
            {
                // Convert to subnormal
                const u32 mantissa16 = (mantissa | 0x800000) >> (1 - exponent16);
                return (sign << 15) | (mantissa16 >> 13);
            }
            else
            {
                // Underflow to zero
                return sign >> 15;
            }
        }
        return (sign << 15) | (exponent16 << 10) | (mantissa >> 13);
#endif
    }

    float Float16::ConvertFromFloat16(u16 _value)
    {
#if defined(KE_ARM_NEON)
        float16x4_t val { *reinterpret_cast<float16_t*>(&_value) };
        float32x4_t result = vcvt_f32_f16(val);
        return *reinterpret_cast<float*>(&result);
#else
        // Extract components from 16-bit float
        const u32 sign = (_value >> 15) & 0x1;
        u32 exponent = (_value >> 10) & 0x1F;
        u32 mantissa = _value & 0x3FF;

        u32 result = 0;

        if (exponent == 0) {
            // Zero or subnormal
            if (mantissa == 0) {
                // Zero
                result = sign << 31;
            } else {
                // Subnormal, normalize it
                int e = -14;
                while ((mantissa & 0x400) == 0) {
                    mantissa <<= 1;
                    e--;
                }
                mantissa &= 0x3FF;
                exponent = e + 15;

                // Convert to float32
                result = (sign << 31) | ((exponent + 127 - 15) << 23) | (mantissa << 13);
            }
        } else if (exponent == 0x1F) {
            // Infinity or NaN
            result = (sign << 31) | (0xFF << 23);
            if (mantissa != 0) {
                // NaN - preserve mantissa bits, setting at least one bit
                result |= (mantissa << 13) | 0x400000;
            }
        } else {
            // Normal number
            result = (sign << 31) | ((exponent + 127 - 15) << 23) | (mantissa << 13);
        }

        return *reinterpret_cast<float*>(&result);
#endif
    }
}