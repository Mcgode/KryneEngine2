/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#include <cstdint>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <glm/detail/type_vec1.hpp>
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>

 // ReSharper disable CppInconsistentNaming
namespace KryneEngine
{
    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using s8 = int8_t;
    using s16 = int16_t;
    using s32 = int32_t;
    using s64 = int64_t;

    // Vectors

    using float1 = glm::vec<1, float, glm::highp>;
    using float2 = glm::vec<2, float, glm::highp>;
    using float3 = glm::vec<3, float, glm::highp>;
    using float4 = glm::vec<4, float, glm::highp>;

    using int1 = glm::vec<1, s32, glm::highp>;
    using int2 = glm::vec<2, s32, glm::highp>;
    using int3 = glm::vec<3, s32, glm::highp>;
    using int4 = glm::vec<4, s32, glm::highp>;

    using uint1 = glm::vec<1, u32, glm::highp>;
    using uint2 = glm::vec<2, u32, glm::highp>;
    using uint3 = glm::vec<3, u32, glm::highp>;
    using uint4 = glm::vec<4, u32, glm::highp>;

    struct Size16x2
    {
        u16 m_width = 0;
        u16 m_height = 0;
    };
}