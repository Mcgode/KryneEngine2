/**
 * @file
 * @author Max Godefroy
 * @date 14/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/Matrix33.hpp"

#ifndef KE_DEFAULT_MATRIX_ROW_MAJOR
#   define KE_DEFAULT_MATRIX_ROW_MAJOR 1
#endif

namespace KryneEngine
{
    using float3x3 = Math::Matrix33Base<float, sizeof(float), KE_DEFAULT_MATRIX_ROW_MAJOR>;
}