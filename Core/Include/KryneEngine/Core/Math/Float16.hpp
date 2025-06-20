/**
 * @file
 * @author Max Godefroy
 * @date 19/06/2025.
 */

#pragma once

#include <KryneEngine/Core/Common/Types.hpp>

namespace KryneEngine::Math
{
    struct Float16
    {
        Float16() = default;
        Float16(const Float16&) = default;
        Float16& operator=(const Float16&) = default;
        ~Float16() = default;

        explicit Float16(float _value);
        Float16& operator=(float _value);
        explicit operator float() const;

        static u16 ConvertToFloat16(float _value);
        static float ConvertFromFloat16(u16 _value);

        u16 m_data;
    };
}