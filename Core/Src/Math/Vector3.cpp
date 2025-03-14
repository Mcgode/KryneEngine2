/**
 * @file
 * @author Max Godefroy
 * @date 28/11/2024.
 */

#include "KryneEngine/Core/Math/Vector3.hpp"
#include "KryneEngine/Core/Math/Vector3.inl"

#include "KryneEngine/Core/Common/Types.hpp"

namespace KryneEngine::Math
{
    template<typename T, size_t Alignment, class Operator>
    inline Vector3Base<T, Alignment> ApplyOperation(
        const Vector3Base<T, Alignment>& _vecA,
        const Vector3Base<T, Alignment>& _vecB)
    {
        Vector3Base<T, Alignment> result {};

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
    Vector3Base<T, Alignment> Vector3Base<T, Alignment>::operator+(const Vector3Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, AddOperator>(*this, _other);
    }

    template <typename T, size_t Alignment>
    Vector3Base<T, Alignment> Vector3Base<T, Alignment>::operator-(const Vector3Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, SubtractOperator>(*this, _other);
    }
    template <typename T, size_t Alignment>
    Vector3Base<T, Alignment> Vector3Base<T, Alignment>::operator*(const Vector3Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, MultiplyOperator>(*this, _other);
    }
    template <typename T, size_t Alignment>
    Vector3Base<T, Alignment> Vector3Base<T, Alignment>::operator/(const Vector3Base<T, Alignment>& _other) const
    {
        return ApplyOperation<T, Alignment, DivideOperator>(*this, _other);
    }

    template <typename T, size_t Alignment>
    T& Vector3Base<T, Alignment>::operator[](size_t _index)
    {
        return reinterpret_cast<T*>(this)[_index];
    }

    template <typename T, size_t Alignment>
    const T& Vector3Base<T, Alignment>::operator[](size_t _index) const
    {
        return reinterpret_cast<const T*>(this)[_index];
    }

    template <typename T, size_t Alignment>
    bool Vector3Base<T, Alignment>::operator==(const Vector3Base& _other) const
    {
        if constexpr (Alignment == 16)
        {
            xsimd::batch<T, XsimdArch128> vecA = xsimd::load_aligned(&x);
            xsimd::batch<T, XsimdArch128> vecB = xsimd::load_aligned(&_other.x);
            xsimd::batch_bool<T, XsimdArch128> eqVec = xsimd::eq(vecA, vecB);
            return (eqVec.mask() & 0b111) == 0b111;
        }
        else if constexpr (Alignment == 32)
        {
            xsimd::batch<T, XsimdArch256> vecA = xsimd::load_aligned(&x);
            xsimd::batch<T, XsimdArch256> vecB = xsimd::load_aligned(&_other.x);
            xsimd::batch_bool<T, XsimdArch256> eqVec = xsimd::eq(vecA, vecB);
            return (eqVec.mask() & 0b111) == 0b111;
        }
        else
        {
            return x == _other.x && y == _other.y && z == _other.z;
        }
    }


    template <typename T, size_t Alignment>
    void Vector3Base<T, Alignment>::Normalize()
        requires std::is_floating_point_v<T>
    {
        const T length = std::sqrt(Dot(*this, *this));
        if (length > 0.0f)
        {
            x /= length;
            y /= length;
            z /= length;
        }
    }

    template <typename T, size_t Alignment>
    Vector3Base<T, Alignment> Vector3Base<T, Alignment>::Normalized() const
        requires std::is_floating_point_v<T>
    {
        Vector3Base result(*this);
        result.Normalize();
        return result;
    }

    template struct Vector3Base<float>;
    template struct Vector3Base<s32>;
    template struct Vector3Base<u32>;
    template struct Vector3Base<float, 16>;
    template struct Vector3Base<s32, 16>;
    template struct Vector3Base<u32, 16>;
} // namespace KryneEngine::Math
