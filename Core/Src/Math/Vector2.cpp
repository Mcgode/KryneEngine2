/**
 * @file
 * @author Max Godefroy
 * @date 28/11/2024.
 */

#include "KryneEngine/Core/Math/Vector2.hpp"

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Math/XSimdUtils.hpp"

namespace KryneEngine::Math
{
    template<typename T, bool SimdOptimal, class Operator>
    inline Vector2Base<T, SimdOptimal> ApplyOperation(
        const Vector2Base<T, SimdOptimal>& _vecA,
        const Vector2Base<T, SimdOptimal>& _vecB)
    {
        using Vector2 = Vector2Base<T, SimdOptimal>;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector2>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;
            Vector2 result{};
            for (size_t i = 0; i < Operability::kBatchCount; ++i)
            {
                xsimd::batch vecA = XsimdLoad<alignedOps, T, OptimalArch>(_vecA.GetPtr() + i * Operability::kBatchSize);
                xsimd::batch vecB = XsimdLoad<alignedOps, T, OptimalArch>(_vecB.GetPtr() + i * Operability::kBatchSize);
                XsimdStore<alignedOps, T, OptimalArch>(result.GetPtr() + i * Operability::kBatchSize, Operator{}(vecA, vecB));
            }
            return result;
        }
        else
        {
            return Vector2 {
                Operator{}(_vecA.x, _vecB.x),
                Operator{}(_vecA.y, _vecB.y)
            };
        }
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
    Vector2Base<T, SimdOptimal> Vector2Base<T, SimdOptimal>::operator+(const Vector2Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, AddOperator>(*this, _other);
    }

    template <typename T, bool SimdOptimal>
    Vector2Base<T, SimdOptimal> Vector2Base<T, SimdOptimal>::operator-(const Vector2Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, SubtractOperator>(*this, _other);
    }
    template <typename T, bool SimdOptimal>
    Vector2Base<T, SimdOptimal> Vector2Base<T, SimdOptimal>::operator*(const Vector2Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, MultiplyOperator>(*this, _other);
    }
    template <typename T, bool SimdOptimal>
    Vector2Base<T, SimdOptimal> Vector2Base<T, SimdOptimal>::operator/(const Vector2Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, DivideOperator>(*this, _other);
    }

    template <typename T, bool SimdOptimal>
    bool Vector2Base<T, SimdOptimal>::operator==(const Vector2Base& _other) const
    {
        using Vector2 = Vector2Base<T, SimdOptimal>;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector2>;

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
            return x == _other.x && y == _other.y;
        }
    }

    template <typename T, bool SimdOptimal>
    void Vector2Base<T, SimdOptimal>::Normalize()
        requires std::is_floating_point_v<T>
    {
        const T length = std::sqrt(Dot(*this, *this));
        if (length > 0.0f)
        {
            x /= length;
            y /= length;
        }
    }

    template <typename T, bool SimdOptimal>
    Vector2Base<T, SimdOptimal> Vector2Base<T, SimdOptimal>::Normalized() const
        requires std::is_floating_point_v<T>
    {
        Vector2Base result(*this);
        result.Normalize();
        return result;
    }

    template <typename T, bool SimdOptimal>
    T Dot(const Vector2Base<T, SimdOptimal>& _a, const Vector2Base<T, SimdOptimal>& _b)
    {
        using Vector2 = Vector2Base<T, SimdOptimal>;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector2>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;
            T result{};
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
            return _a.x * _b.x + _a.y * _b.y;
        }
    }

#define IMPLEMENT_SIMD(type, simd)                                                                                      \
    template struct Vector2Base<type, simd>;                                                                            \
    template type Dot<type, simd>(const Vector2Base<type, simd>& _a, const Vector2Base<type, simd>& _b)

#define IMPLEMENT(type)                                                                                                 \
    IMPLEMENT_SIMD(type, false);                                                                                        \
    IMPLEMENT_SIMD(type, true)

    IMPLEMENT(float);
    IMPLEMENT(s32);
    IMPLEMENT(u32);
    IMPLEMENT(double);

#undef IMPLEMENT
#undef IMPLEMENT_SIMD
} // namespace KryneEngine::Math
