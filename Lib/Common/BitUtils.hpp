/**
 * @file
 * @author Max Godefroy
 * @date 20/11/2022.
 */

#pragma once

#include <Common/KETypes.hpp>

namespace KryneEngine::BitUtils
{
    template<class T>
    inline T BitMask(u8 _size)
    {
        return ((1 << _size) - 1);
    }

    template<class T>
    inline T BitMask(u8 _size, u8 _offset)
    {
        return ((1 << _size) - 1) << _offset;
    }
}