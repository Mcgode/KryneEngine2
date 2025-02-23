/**
 * @file
 * @author Max Godefroy
 * @date 28/11/2024.
 */

#pragma once

#include <xsimd/xsimd.hpp>

namespace KryneEngine::Math
{
#if XSMID_WITH_SSE4_2
    using XsimdArch128 = xsimd::sse4_2;
#elif XSMID_WITH_SSE4_1
    using XsimdArch128 = xsimd::sse4_1;
#elif XSMID_WITH_SSE3
    using XsimdArch128 = xsimd::sse3;
#elif XSIMD_WITH_SSE2
    using XsimdArch128 = xsimd::sse2;
#elif XSIMD_WITH_SSE
    using XsimdArch128 = xsimd::sse;
#elif XSIMD_WITH_NEON64
    using XsimdArch128 = xsimd::neon64;
#elif XSIMD_WITH_NEON
    using XsimdArch128 = xsimd::neon;
#else
    // Fall back to basic scalar operations
    using XsimdArch128 = xsimd::generic;
#endif

#if XSIMD_WITH_AVX
    using XsimdArch256 = xsimd::avx;
#elif XSIMD_WITH_SVE
    using XsimdArch256 = xsimd::sve<256>;
#elif XSIMD_WITH_RVV
    using XsimdArch256 = xsimd::rvv<256>;
#else
    // Fall back to 128 bits SIMD
    using XsimdArch256 = XsimdArch128;
#endif

#if XSIMD_WITH_AVX512F
    using XsimdArch512 = xsimd::avx512f;
#elif XSIMD_WITH_SVE
    using XsimdArch512 = xsimd::sve<512>;
#elif XSIMD_WITH_RVV
    using XsimdArch512 = xsimd::rvv<512>;
#else
    // Fall back to 256 bits SIMD
    using XsimdArch512 = XsimdArch256;
#endif
}
