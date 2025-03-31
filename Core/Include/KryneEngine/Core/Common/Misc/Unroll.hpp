/**
 * @file
 * @author Max Godefroy
 * @date 31/03/2025.
 */

#pragma once

#include <cstddef>
#include <EABase/config/eacompilertraits.h>

namespace KryneEngine
{
    template <size_t To, size_t Index = 0, class Func>
        requires (To <= Index)
    EA_FORCE_INLINE void Unroll(Func&&)
    {}

    template <size_t To, size_t Index = 0, class Func>
    requires (To > Index)
    EA_FORCE_INLINE void Unroll(Func&& _func)
    {
        _func(Index);
        Unroll<To, Index + 1>(_func);
    }

#define UNROLL_FOR_LOOP(IndexVar, To) Unroll<To>([&](auto IndexVar) {
#define END_UNROLL() });
}