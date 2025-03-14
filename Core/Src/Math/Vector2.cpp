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
    template<typename T, size_t Alignment, class Operator>
    inline Vector2Base<T, Alignment> ApplyOperation(
        const Vector2Base<T, Alignment>& _vecA,
        const Vector2Base<T, Alignment>& _vecB)
    {
        Vector2Base<T, Alignment> result {};

        if constexpr (Alignment == 16)
        {
            xsimd::batch<T, XsimdArch128> vecA = xsimd::load_aligned(&_vecA.x);
            xsimd::batch<T, XsimdArch128> vecB = xsimd::load_aligned(&_vecB.x);
            xsimd::batch<T, XsimdArch128> res = Operator{}(vecA, vecB);
            res.store_aligned(&result.x);
        }
        else
        {
            if constexpr (sizeof(T) == 4)
            {
                xsimd::batch<T, XsimdArch128> vecA = xsimd::load_unaligned(&_vecA.x);
                xsimd::batch<T, XsimdArch128> vecB = xsimd::load_unaligned(&_vecB.x);
                xsimd::batch<T, XsimdArch128> res = Operator{}(vecA, vecB);
                res.store_unaligned(&result.x);
            }
            else
            {
                result.x = Operator{}(_vecA.x, _vecB.x);
                result.y = Operator{}(_vecA.y, _vecB.y);
            }
        }
        return std::move(result);
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

    template <typename T, size_t Alignment>
    Vector2Base<T, Alignment> Vector2Base<T, Alignment>::operator+(const Vector2Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, AddOperator>(*this, _other);
    }

    template <typename T, size_t Alignment>
    Vector2Base<T, Alignment> Vector2Base<T, Alignment>::operator-(const Vector2Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, SubtractOperator>(*this, _other);
    }
    template <typename T, size_t Alignment>
    Vector2Base<T, Alignment> Vector2Base<T, Alignment>::operator*(const Vector2Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, MultiplyOperator>(*this, _other);
    }
    template <typename T, size_t Alignment>
    Vector2Base<T, Alignment> Vector2Base<T, Alignment>::operator/(const Vector2Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, DivideOperator>(*this, _other);
    }

    template <typename T, size_t Alignment>
    bool Vector2Base<T, Alignment>::operator==(const Vector2Base& _other) const
    {
        if constexpr (Alignment == 16)
        {
            xsimd::batch<T, XsimdArch128> vecA = xsimd::load_aligned(&x);
            xsimd::batch<T, XsimdArch128> vecB = xsimd::load_aligned(&_other.x);
            return xsimd::all(xsimd::eq(vecA, vecB));
        }
        else
        {
            if constexpr (sizeof(T) == 8)
            {
                xsimd::batch<T, XsimdArch128> vecA = xsimd::load_unaligned(&x);
                xsimd::batch<T, XsimdArch128> vecB = xsimd::load_unaligned(&_other.x);
                return xsimd::all(xsimd::eq(vecA, vecB));
            }
            else
            {
                return x == _other.x && y == _other.y;
            }
        }
    }

    template <typename T, size_t Alignment>
    void Vector2Base<T, Alignment>::Normalize()
        requires std::is_floating_point_v<T>
    {
        const T length = std::sqrt(Dot(*this, *this));
        if (length > 0.0f)
        {
            x /= length;
            y /= length;
        }
    }

    template <typename T, size_t Alignment>
    Vector2Base<T, Alignment> Vector2Base<T, Alignment>::Normalized() const
        requires std::is_floating_point_v<T>
    {
        Vector2Base result(*this);
        result.Normalize();
        return result;
    }

    template <typename T, size_t Alignment>
    T Dot(const Vector2Base<T, Alignment>& _a, const Vector2Base<T, Alignment>& _b)
    {
        if constexpr (Alignment == 16)
        {
            xsimd::batch<T, XsimdArch128> vecA = xsimd::load_aligned(&_a.x);
            xsimd::batch<T, XsimdArch128> vecB = xsimd::load_aligned(&_b.x);
            return xsimd::reduce_add(xsimd::mul(vecA, vecB));
        }
        else
        {
            if constexpr (sizeof(T) == 8)
            {
                xsimd::batch<T, XsimdArch128> vecA = xsimd::load_unaligned(&_a.x);
                xsimd::batch<T, XsimdArch128> vecB = xsimd::load_unaligned(&_b.x);
                return xsimd::reduce_add(xsimd::mul(vecA, vecB));
            }
            else
            {
                return _a.x * _b.x + _a.y * _b.y;
            }
        }
    }

    template struct Vector2Base<float>;
    template struct Vector2Base<s32>;
    template struct Vector2Base<u32>;
    template struct Vector2Base<float, 16>;
    template struct Vector2Base<s32, 16>;
    template struct Vector2Base<u32, 16>;
} // namespace KryneEngine::Math
