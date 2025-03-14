/**
 * @file
 * @author Max Godefroy
 * @date 28/11/2024.
 */

#include "KryneEngine/Core/Math/Vector4.hpp"

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Math/XSimdUtils.hpp"

namespace KryneEngine::Math
{
    template<typename T, bool SimdOptimal, class Operator>
    inline Vector4Base<T, SimdOptimal> ApplyOperation(
        const Vector4Base<T, SimdOptimal>& _vecA,
        const Vector4Base<T, SimdOptimal>& _vecB)
    {
        using Vector4 = Vector4Base<T, SimdOptimal>;
        Vector4 result {};

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector4>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;
            for (size_t i = 0; i < Operability::kBatchCount; ++i)
            {
                xsimd::batch vecA = XsimdLoad<alignedOps, T, OptimalArch>(_vecA.GetPtr() + i * Operability::kBatchSize);
                xsimd::batch vecB = XsimdLoad<alignedOps, T, OptimalArch>(_vecB.GetPtr() + i * Operability::kBatchSize);
                xsimd::batch res = Operator{}(vecA, vecB);
                XsimdStore<alignedOps, T, OptimalArch>(result.GetPtr() + i * Operability::kBatchSize, res);
            }
        }
        else
        {
            result.x = Operator{}(_vecA.x, _vecB.x);
            result.y = Operator{}(_vecA.y, _vecB.y);
            result.z = Operator{}(_vecA.z, _vecB.z);
            result.w = Operator{}(_vecA.w, _vecB.w);
        }

        return result;
    }

    struct AddOperator
    {
        template <class T>
        T operator()(const T& _a, const T& _b) { return _a + _b; }
    };

    struct SubtractOperator
    {
        template <class T>
        T operator()(const T& _a, const T& _b)  { return _a - _b; }
    };

    struct MultiplyOperator
    {
        template <class T>
        T operator()(const T& _a, const T& _b) { return _a * _b; }
    };

    struct DivideOperator
    {
        template <class T>
        T operator()(const T& _a, const T& _b) { return _a / _b; }
    };

    template <typename T, bool SimdOptimal>
    Vector4Base<T, SimdOptimal> Vector4Base<T, SimdOptimal>::operator+(const Vector4Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, AddOperator>(*this, _other);
    }

    template <typename T, bool SimdOptimal>
    Vector4Base<T, SimdOptimal> Vector4Base<T, SimdOptimal>::operator-(const Vector4Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, SubtractOperator>(*this, _other);
    }
    template <typename T, bool SimdOptimal>
    Vector4Base<T, SimdOptimal> Vector4Base<T, SimdOptimal>::operator*(const Vector4Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, MultiplyOperator>(*this, _other);
    }
    template <typename T, bool SimdOptimal>
    Vector4Base<T, SimdOptimal> Vector4Base<T, SimdOptimal>::operator/(const Vector4Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, DivideOperator>(*this, _other);
    }

    template <typename T, bool SimdOptimal>
    T& Vector4Base<T, SimdOptimal>::operator[](size_t _index)
    {
        _index = eastl::min<size_t>(3u, _index);
        return (&x)[_index];
    }

    template <typename T, bool SimdOptimal>
    const T& Vector4Base<T, SimdOptimal>::operator[](size_t _index) const
    {
        _index = eastl::min<size_t>(3u, _index);
        return (&x)[_index];
    }

    template <typename T, bool SimdOptimal>
    T* Vector4Base<T, SimdOptimal>::GetPtr()
    {
        return &x;
    }

    template <typename T, bool SimdOptimal>
    const T* Vector4Base<T, SimdOptimal>::GetPtr() const
    {
        return &x;
    }

    template <typename T, bool SimdOptimal>
    bool Vector4Base<T, SimdOptimal>::operator==(const Vector4Base& _other) const
    {
        using Vector4 = Vector4Base<T, SimdOptimal>;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector4>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;
            bool result = true;
            for (size_t i = 0; i < Operability::kBatchCount; ++i)
            {
                xsimd::batch vecA = XsimdLoad<alignedOps, T, OptimalArch>(GetPtr() + i * Operability::kBatchSize);
                xsimd::batch vecB = XsimdLoad<alignedOps, T, OptimalArch>(_other.GetPtr() + i * Operability::kBatchSize);
                result &= xsimd::all(xsimd::eq(vecA, vecB));
            }
            return result;
        }
        else
        {
            return x == _other.x && y == _other.y && z == _other.z && w == _other.w;
        }
    }

    template <typename T, bool SimdOptimal>
    void Vector4Base<T, SimdOptimal>::Normalize()
        requires std::is_floating_point_v<T>
    {
        const T length = std::sqrt(Dot(*this, *this));
        if (length > 0.0f)
        {
            x /= length;
            y /= length;
            z /= length;
            w /= length;
        }
    }

    template <typename T, bool SimdOptimal>
    Vector4Base<T, SimdOptimal> Vector4Base<T, SimdOptimal>::Normalized() const
        requires std::is_floating_point_v<T>
    {
        Vector4Base result(*this);
        result.Normalize();
        return result;
    }

    template <typename T, bool SimdOptimal>
    T Dot(const Vector4Base<T, SimdOptimal>& _a, const Vector4Base<T, SimdOptimal>& _b)
    {
        using Vector4 = Vector4Base<T, SimdOptimal>;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector4>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;
            T result {};
            for (size_t i = 0; i < Operability::kBatchCount; ++i)
            {
                xsimd::batch vecA = XsimdLoad<alignedOps, T, OptimalArch>(_a.GetPtr() + i * Operability::kBatchSize);
                xsimd::batch vecB = XsimdLoad<alignedOps, T, OptimalArch>(_b.GetPtr() + i * Operability::kBatchSize);
                result += xsimd::reduce_add(xsimd::mul(vecA, vecB));
            }
            return result;
        }
        else
        {
            return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z + _a.w * _b.w;
        }
    }

#define IMPLEMENT(type)                                                                                                 \
    template struct Vector4Base<type>;                                                                                  \
    template struct Vector4Base<type, true>;                                                                            \
    template type Dot<type, false>(const Vector4Base<type, false>& _a, const Vector4Base<type, false>& _b);                         \
    template type Dot<type, true>(const Vector4Base<type, true>& _a, const Vector4Base<type, true>& _b)
    
    IMPLEMENT(float);
    IMPLEMENT(s32);
    IMPLEMENT(u32);
    IMPLEMENT(double);
} // namespace KryneEngine::Math
