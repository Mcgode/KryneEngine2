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
    template<typename T, size_t Alignment, class Operator>
    inline Vector4Base<T, Alignment> ApplyOperation(
        const Vector4Base<T, Alignment>& _vecA,
        const Vector4Base<T, Alignment>& _vecB)
    {
        Vector4Base<T, Alignment> result {};

        if constexpr (Alignment == 16)
        {
            xsimd::batch<T, XsimdArch128> vecA = xsimd::load_aligned(&_vecA.x);
            xsimd::batch<T, XsimdArch128> vecB = xsimd::load_aligned(&_vecB.x);
            xsimd::batch<T, XsimdArch128> res = Operator{}(vecA, vecB);
            res.store_aligned(&result.x);
        }
        else if constexpr (Alignment == 32)
        {
            xsimd::batch<T, XsimdArch256> vecA = xsimd::load_aligned(&_vecA.x);
            xsimd::batch<T, XsimdArch256> vecB = xsimd::load_aligned(&_vecB.x);
            xsimd::batch<T, XsimdArch256> res = Operator{}(vecA, vecB);
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
            else if constexpr (sizeof(T) == 8)
            {
                xsimd::batch<T, XsimdArch256> vecA = xsimd::load_unaligned(&_vecA.x);
                xsimd::batch<T, XsimdArch256> vecB = xsimd::load_unaligned(&_vecB.x);
                xsimd::batch<T, XsimdArch256> res = Operator{}(vecA, vecB);
                res.store_unaligned(&result.x);
            }
            else
            {
                result.x = Operator{}(_vecA.x, _vecB.x);
                result.z = Operator{}(_vecA.y, _vecB.y);
                result.y = Operator{}(_vecA.z, _vecB.z);
                result.w = Operator{}(_vecA.w, _vecB.w);
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
    Vector4Base<T, Alignment> Vector4Base<T, Alignment>::operator+(const Vector4Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, AddOperator>(*this, _other);
    }

    template <typename T, size_t Alignment>
    Vector4Base<T, Alignment> Vector4Base<T, Alignment>::operator-(const Vector4Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, SubtractOperator>(*this, _other);
    }
    template <typename T, size_t Alignment>
    Vector4Base<T, Alignment> Vector4Base<T, Alignment>::operator*(const Vector4Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, MultiplyOperator>(*this, _other);
    }
    template <typename T, size_t Alignment>
    Vector4Base<T, Alignment> Vector4Base<T, Alignment>::operator/(const Vector4Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, DivideOperator>(*this, _other);
    }

    template <typename T, size_t Alignment>
    T& Vector4Base<T, Alignment>::operator[](size_t _index)
    {
        _index = eastl::min<size_t>(3u, _index);
        return (&x)[_index];
    }

    template <typename T, size_t Alignment>
    const T& Vector4Base<T, Alignment>::operator[](size_t _index) const
    {
        _index = eastl::min<size_t>(3u, _index);
        return (&x)[_index];
    }

    template <typename T, size_t Alignment>
    T* Vector4Base<T, Alignment>::GetPtr()
    {
        return &x;
    }

    template <typename T, size_t Alignment>
    const T* Vector4Base<T, Alignment>::GetPtr() const
    {
        return &x;
    }

    template <typename T, size_t Alignment>
    bool Vector4Base<T, Alignment>::operator==(const Vector4Base& _other) const
    {
        if constexpr (Alignment == 16)
        {
            xsimd::batch<T, XsimdArch128> vecA = xsimd::load_aligned(&x);
            xsimd::batch<T, XsimdArch128> vecB = xsimd::load_aligned(&_other.x);
            return xsimd::all(xsimd::eq(vecA, vecB));
        }
        else if constexpr (Alignment == 32)
        {
            xsimd::batch<T, XsimdArch256> vecA = xsimd::load_aligned(&x);
            xsimd::batch<T, XsimdArch256> vecB = xsimd::load_aligned(&_other.x);
            return xsimd::all(xsimd::eq(vecA, vecB));
        }
        else
        {
            if constexpr (sizeof(T) == 4)
            {
                xsimd::batch<T, XsimdArch128> vecA = xsimd::load_unaligned(&x);
                xsimd::batch<T, XsimdArch128> vecB = xsimd::load_unaligned(&_other.x);
                return xsimd::all(xsimd::eq(vecA, vecB));
            }
            else if constexpr (sizeof(T) == 8)
            {
                xsimd::batch<T, XsimdArch256> vecA = xsimd::load_unaligned(&x);
                xsimd::batch<T, XsimdArch256> vecB = xsimd::load_unaligned(&_other.x);
                return xsimd::all(xsimd::eq(vecA, vecB));
            }
            else
            {
                return x == _other.x && y == _other.y && z == _other.z && w == _other.w;
            }
        }
    }

    template <typename T, size_t Alignment>
    void Vector4Base<T, Alignment>::Normalize()
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

    template <typename T, size_t Alignment>
    Vector4Base<T, Alignment> Vector4Base<T, Alignment>::Normalized() const
        requires std::is_floating_point_v<T>
    {
        Vector4Base result(*this);
        result.Normalize();
        return result;
    }

    template <typename T, size_t Alignment>
    T Dot(const Vector4Base<T, Alignment>& _a, const Vector4Base<T, Alignment>& _b)
    {
        if constexpr (Alignment == 16)
        {
            xsimd::batch<T, XsimdArch128> vecA = xsimd::load_aligned(&_a.x);
            xsimd::batch<T, XsimdArch128> vecB = xsimd::load_aligned(&_b.x);
            return xsimd::reduce_add(xsimd::mul(vecA, vecB));
        }
        else if constexpr (Alignment == 32)
        {
            xsimd::batch<T, XsimdArch256> vecA = xsimd::load_aligned(&_a.x);
            xsimd::batch<T, XsimdArch256> vecB = xsimd::load_aligned(&_b.x);
            return xsimd::reduce_add(xsimd::mul(vecA, vecB));
        }
        else
        {
            if constexpr (sizeof(T) == 4)
            {
                xsimd::batch<T, XsimdArch128> vecA = xsimd::load_unaligned(&_a.x);
                xsimd::batch<T, XsimdArch128> vecB = xsimd::load_unaligned(&_b.x);
                return xsimd::reduce_add(xsimd::mul(vecA, vecB));
            }
            else if constexpr (sizeof(T) == 8)
            {
                xsimd::batch<T, XsimdArch256> vecA = xsimd::load_unaligned(&_a.x);
                xsimd::batch<T, XsimdArch256> vecB = xsimd::load_unaligned(&_b.x);
                return xsimd::reduce_add(xsimd::mul(vecA, vecB));
            }
            else
            {
                return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z + _a.w * _b.w;
            }
        }
    }

    template struct Vector4Base<float>;
    template struct Vector4Base<s32>;
    template struct Vector4Base<u32>;
    template struct Vector4Base<float, 16>;
    template struct Vector4Base<s32, 16>;
    template struct Vector4Base<u32, 16>;
} // namespace KryneEngine::Math
