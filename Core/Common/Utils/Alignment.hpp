/**
 * @file
 * @author Max Godefroy
 * @date 29/07/2024.
 */

#pragma once

#include <Common/BitUtils.hpp>

namespace KryneEngine::Alignment
{
    template <class T>
    inline T AlignUp(T _value, T _alignment)
    {
        return ((_value + _alignment - 1) / _alignment) * _alignment;
    }

    template <class T>
    inline T AlignUpPot(T _value, u8 _pot)
    {
        return ((_value + BitUtils::BitMask<T>(_pot)) >> _pot) << _pot;
    }
}
