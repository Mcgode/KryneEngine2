/**
 * @file
 * @author Max Godefroy
 * @date 28/11/2024.
 */

#pragma once

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Math/Vector2.hpp"
#include "KryneEngine/Core/Math/Vector3.hpp"
#include "KryneEngine/Core/Math/Vector4.hpp"

namespace KryneEngine
{
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"

    using float2 = Math::Vector2Base<float>;
    using int2 = Math::Vector2Base<s32>;
    using uint2 = Math::Vector2Base<u32>;

    using float3 = Math::Vector3Base<float>;
    using int3 = Math::Vector3Base<s32>;
    using uint3 = Math::Vector3Base<u32>;

    using float4 = Math::Vector4Base<float>;
    using int4 = Math::Vector4Base<s32>;
    using uint4 = Math::Vector4Base<u32>;

    using float2_simd = Math::Vector2Base<float, 16>;
    using int2_simd = Math::Vector2Base<s32, 16>;
    using uint2_simd = Math::Vector2Base<u32, 16>;

    using float3_simd = Math::Vector3Base<float, 16>;
    using int3_simd = Math::Vector3Base<s32, 16>;
    using uint3_simd = Math::Vector3Base<u32, 16>;

    using float4_simd = Math::Vector4Base<float, 16>;
    using int4_simd = Math::Vector4Base<s32, 16>;
    using uint4_simd = Math::Vector4Base<u32, 16>;

#pragma clang diagnostic pop
}