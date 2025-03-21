/**
 * @file
 * @author Max Godefroy
 * @date 14/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/Matrix33.hpp"
#include "KryneEngine/Core/Math/Matrix44.hpp"

#ifndef KE_DEFAULT_MATRIX_ROW_MAJOR
#   define KE_DEFAULT_MATRIX_ROW_MAJOR 1
#endif

namespace KryneEngine
{
    using float3x3 = Math::Matrix33Base<float, false, KE_DEFAULT_MATRIX_ROW_MAJOR>;
    using double3x3 = Math::Matrix33Base<double, false, KE_DEFAULT_MATRIX_ROW_MAJOR>;
    using float3x3_simd = Math::Matrix33Base<float, true, KE_DEFAULT_MATRIX_ROW_MAJOR>;
    using double3x3_simd = Math::Matrix33Base<double, true, KE_DEFAULT_MATRIX_ROW_MAJOR>;

    using float4x4 = Math::Matrix44Base<float, false, KE_DEFAULT_MATRIX_ROW_MAJOR>;
    using double4x4 = Math::Matrix44Base<double, false, KE_DEFAULT_MATRIX_ROW_MAJOR>;
    using float4x4_simd = Math::Matrix44Base<float, true, KE_DEFAULT_MATRIX_ROW_MAJOR>;
    using double4x4_simd = Math::Matrix44Base<double, true, KE_DEFAULT_MATRIX_ROW_MAJOR>;
}