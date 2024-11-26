/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include "KryneEngine/Core/Common/Arrays.hpp"

namespace KryneEngine
{
    class FibersManager;

    template<class T>
    struct FiberTls
    {
    public:
        using Type = T;
        using Ptr = typename DynamicArray<T>::Ptr;
        using Ref = typename DynamicArray<T>::Ref;

        void Init(const FibersManager *_fibersManager, const T &_value);

        template <typename F>
        void InitFunc(const FibersManager* _fibersManager, F _initFunction);

        [[nodiscard]] T& Load();

        [[nodiscard]] inline T& Load(u16 _fiberIndex)
        {
            return m_array[_fiberIndex];
        }

    private:
        DynamicArray<T> m_array {};
    };
} // KryneEngine