/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/Vector3.hpp"

#include "KryneEngine/Core/Math/XSimdUtils.hpp"

namespace KryneEngine::Math
{
    template <typename T, size_t Alignment>
    T Dot(const Vector3Base<T, Alignment>& _a, const Vector3Base<T, Alignment>& _b)
    {
        if constexpr (Alignment == 16)
        {
            xsimd::batch<T, XsimdArch128> vecA = xsimd::load_aligned(&_a.x);
            xsimd::batch<T, XsimdArch128> vecB = xsimd::load_aligned(&_b.x);
            return xsimd::reduce_add(xsimd::mul(vecA, vecB));
        }
        else if constexpr (Alignment == 32)
        {
            xsimd::batch<T, XsimdArch256> vecA = xsimd::load_aligned(&_a.x);
            xsimd::batch<T, XsimdArch256> vecB = xsimd::load_aligned(&_b.x);
            return xsimd::reduce_add(xsimd::mul(vecA, vecB));
        }
        else
        {
            return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z;
        }
    }

    template <typename T, size_t Alignment>
    Vector3Base<T, Alignment> CrossProduct(const Vector3Base<T, Alignment>& _a, const Vector3Base<T, Alignment>& _b)
    {
        return Vector3Base<T, Alignment> {
            _a.y * _b.z - _a.z * _b.y,
            _a.z * _b.x - _a.x * _b.z,
            _a.x * _b.y - _a.y * _b.x
        };
    }
}