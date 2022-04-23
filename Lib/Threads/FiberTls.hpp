/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include <Common/Arrays.hpp>

namespace KryneEngine
{
    template<class T>
    struct FiberTLS
    {
    public:
        void Init();
        [[nodiscard]] T& Load();

        [[nodiscard]] inline T& Load(u16 _fiberIndex)
        {
            return m_array[_fiberIndex];
        }

    private:
        DynamicArray<T> m_array {};
    };
} // KryneEngine