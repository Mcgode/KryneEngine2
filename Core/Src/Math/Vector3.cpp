/**
 * @file
 * @author Max Godefroy
 * @date 28/11/2024.
 */

#include "KryneEngine/Core/Math/Vector3.hpp"

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Math/XSimdUtils.hpp"

namespace KryneEngine::Math
{
    template<typename T, bool SimdOptimal, class Operator>
    inline Vector3Base<T, SimdOptimal> ApplyOperation(
        const Vector3Base<T, SimdOptimal>& _vecA,
        const Vector3Base<T, SimdOptimal>& _vecB)
    {
        using Vector3 = Vector3Base<T, SimdOptimal>;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector3>;

        if constexpr (Operability::kSimdOperable)
        {
            using OptimalArch = Operability::OptimalArch;
            Vector3Base<T, SimdOptimal> result {};
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
            return {
                Operator{}(_vecA.x, _vecB.x),
                Operator{}(_vecA.y, _vecB.y),
                Operator{}(_vecA.z, _vecB.z)
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
    Vector3Base<T, SimdOptimal> Vector3Base<T, SimdOptimal>::operator+(const Vector3Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, AddOperator>(*this, _other);
    }

    template <typename T, bool SimdOptimal>
    Vector3Base<T, SimdOptimal> Vector3Base<T, SimdOptimal>::operator-(const Vector3Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, SubtractOperator>(*this, _other);
    }
    template <typename T, bool SimdOptimal>
    Vector3Base<T, SimdOptimal> Vector3Base<T, SimdOptimal>::operator*(const Vector3Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, MultiplyOperator>(*this, _other);
    }
    template <typename T, bool SimdOptimal>
    Vector3Base<T, SimdOptimal> Vector3Base<T, SimdOptimal>::operator/(const Vector3Base<T, SimdOptimal>& _other) const
    {
        return ApplyOperation<T, SimdOptimal, DivideOperator>(*this, _other);
    }

    template <typename T, bool SimdOptimal>
    T& Vector3Base<T, SimdOptimal>::operator[](size_t _index)
    {
        return reinterpret_cast<T*>(this)[_index];
    }

    template <typename T, bool SimdOptimal>
    const T& Vector3Base<T, SimdOptimal>::operator[](size_t _index) const
    {
        return reinterpret_cast<const T*>(this)[_index];
    }

    template <typename T, bool SimdOptimal>
    bool Vector3Base<T, SimdOptimal>::operator==(const Vector3Base& _other) const
    {
        using Vector3 = Vector3Base<T, SimdOptimal>;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector3>;

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
            return x == _other.x && y == _other.y && z == _other.z;
        }
    }

    template <typename T, bool SimdOptimal>
    T Vector3Base<T, SimdOptimal>::LengthSquared() const
    {
        return Dot(*this, *this);
    }

    template <typename T, bool SimdOptimal>
    T Vector3Base<T, SimdOptimal>::Length() const
    {
        return sqrt(LengthSquared());
    }

    template <typename T, bool SimdOptimal>
    void Vector3Base<T, SimdOptimal>::Normalize()
        requires std::is_floating_point_v<T>
    {
        const T length = Length();
        if (length > 0.0f)
        {
            x /= length;
            y /= length;
            z /= length;
        }
    }

    template <typename T, bool SimdOptimal>
    Vector3Base<T, SimdOptimal> Vector3Base<T, SimdOptimal>::Normalized() const
        requires std::is_floating_point_v<T>
    {
        Vector3Base result(*this);
        result.Normalize();
        return result;
    }

    template <typename T, bool SimdOptimal>
    T Dot(const Vector3Base<T, SimdOptimal>& _a, const Vector3Base<T, SimdOptimal>& _b)
    {
        using Vector3 = Vector3Base<T, SimdOptimal>;

        constexpr bool alignedOps = SimdOptimal;
        using Operability = SimdOperability<T, Vector3>;

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
            return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z;
        }
    }

    template <typename T, bool SimdOptimal>
    Vector3Base<T, SimdOptimal> CrossProduct(const Vector3Base<T, SimdOptimal>& _a, const Vector3Base<T, SimdOptimal>& _b)
    {
        return Vector3Base<T, SimdOptimal> {
            _a.y * _b.z - _a.z * _b.y,
            _a.z * _b.x - _a.x * _b.z,
            _a.x * _b.y - _a.y * _b.x
        };
    }

#define IMPLEMENT(type)                                                                                                 \
    template struct Vector3Base<type>;                                                                                  \
    template struct Vector3Base<type, true>;                                                                            \
    template type Dot<type, false>(const Vector3Base<type, false>& _a, const Vector3Base<type, false>& _b);             \
    template type Dot<type, true>(const Vector3Base<type, true>& _a, const Vector3Base<type, true>& _b);                \
    template Vector3Base<type, false> CrossProduct<type, false>(const Vector3Base<type, false>& _a, const Vector3Base<type, false>& _b); \
    template Vector3Base<type, true> CrossProduct<type, true>(const Vector3Base<type, true>& _a, const Vector3Base<type, true>& _b)

    IMPLEMENT(float);
    IMPLEMENT(s32);
    IMPLEMENT(u32);
    IMPLEMENT(double);

#undef IMPLEMENT
} // namespace KryneEngine::Math
