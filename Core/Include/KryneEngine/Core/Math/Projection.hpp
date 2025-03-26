/**
 * @file
 * @author Max Godefroy
 * @date 23/03/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/Matrix44.hpp"
#include "KryneEngine/Core/Math/CoordinateSystem.hpp"

namespace KryneEngine::Math
{
    template<Matrix44Type Mat44, CoordinateSystem CS = kDefaultCoordinateSystem>
    Mat44 PerspectiveProjection(float _fov, float _aspect, float _near, float _far, bool _reversedDepth)
    {
        // Based on https://iolite-engine.com/blog_posts/reverse_z_cheatsheet

        const float e = 1.0f / tanf(_fov * 0.5f);
        const float n = _near;
        const float f = _far;

        Mat44 baseMat {
            e / _aspect, 0.f, 0.f, 0.f,
            0.f,         0.f, 0.f, 0.f,
            0.f,         0.f, 0.f, 0.f,
            0.f,         0.f, 0.f, 0.f,
        };

        constexpr size_t projYCol = IsZUp(CS) ? 2 : 1;

        baseMat.Get(1, projYCol) = e;

        constexpr size_t projZCol = IsZUp(CS) ? 1 : 2;
        constexpr bool thirdAxisForward = IsZUp(CS) ^ IsLeftHanded(CS);

        baseMat.Get(3, projZCol) = thirdAxisForward ? 1 : -1;

        if (_far == INFINITY)
        {
            {
                const float value = _reversedDepth ? 0 : 1;
                baseMat.Get(2, projZCol) = thirdAxisForward ? value : -value;
            }
            {
                baseMat.Get(2, 3) = _reversedDepth ? n : -n;
            }
        }
        else
        {
            {
                const float num = _reversedDepth ? -n : f;
                baseMat.Get(2, projZCol) = (thirdAxisForward ? num : -num) / (f - n);
            }
            {
                const float num = _reversedDepth ? f * n : -f * n;
                baseMat.Get(2, 3) = num / (f - n);
            }
        }

        return baseMat;
    }
}