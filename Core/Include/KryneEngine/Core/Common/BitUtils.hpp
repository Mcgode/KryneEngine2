/**
 * @file
 * @author Max Godefroy
 * @date 20/11/2022.
 */

#pragma once

#include <type_traits>
#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Common/Assert.hpp"

#define KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(EnumType) inline                                                          \
constexpr EnumType operator&(EnumType _a, EnumType _b)                                                                \
{                                                                                                                     \
    return static_cast<EnumType>(                                                                                     \
        static_cast<std::underlying_type_t<EnumType>>(_a)                                                             \
        & static_cast<std::underlying_type_t<EnumType>>(_b));                                                         \
}                                                                                                                     \
                                                                                                                      \
inline EnumType& operator&=(EnumType& _a, EnumType _b)                                                                \
{                                                                                                                     \
    _a = _a & _b;                                                                                                     \
    return _a;                                                                                                        \
}                                                                                                                     \
                                                                                                                      \
inline constexpr EnumType operator|(EnumType _a, EnumType _b)                                                         \
{                                                                                                                     \
    return static_cast<EnumType>(                                                                                     \
        static_cast<std::underlying_type_t<EnumType>>(_a)                                                             \
        | static_cast<std::underlying_type_t<EnumType>>(_b));                                                         \
}                                                                                                                     \
                                                                                                                      \
inline EnumType& operator|=(EnumType& _a, EnumType _b)                                                                \
{                                                                                                                     \
    _a = _a | _b;                                                                                                     \
    return _a;                                                                                                        \
}                                                                                                                     \
                                                                                                                      \
inline constexpr EnumType operator^(EnumType _a, EnumType _b)                                                         \
{                                                                                                                     \
    return static_cast<EnumType>(                                                                                     \
        static_cast<std::underlying_type_t<EnumType>>(_a)                                                             \
        ^ static_cast<std::underlying_type_t<EnumType>>(_b));                                                         \
}                                                                                                                     \
                                                                                                                      \
inline EnumType& operator^=(EnumType& _a, EnumType _b)                                                                \
{                                                                                                                     \
    _a = _a ^ _b;                                                                                                     \
    return _a;                                                                                                        \
}                                                                                                                     \
                                                                                                                      \
inline constexpr EnumType operator~(EnumType _value)                                                                  \
{                                                                                                                     \
    return static_cast<EnumType>(~static_cast<std::underlying_type_t<EnumType>>(_value));                             \
}

namespace KryneEngine::BitUtils
{
    template<class T>
    inline constexpr T BitMask(u8 _size)
    {
        return ((1ull << T(_size)) - 1);
    }

    template<class T>
    inline constexpr T BitMask(u8 _size, u8 _offset)
    {
        return ((1 << _size) - 1) << _offset;
    }

    template<class TrueType, u8 Size, u8 Offset>
    struct BitFieldMember
    {
        TrueType m_value;

        static constexpr TrueType One() { return 1 << Offset; }
        static constexpr TrueType Maximum() { return BitMask<TrueType>(Size); }

        [[nodiscard]] inline bool IsZero() const
        {
            return (m_value & BitMask<TrueType>(Size, Offset)) == 0;
        };

        operator TrueType() const
        {
            return m_value >> Offset & Maximum();
        }

        inline BitFieldMember& operator=(TrueType _v)
        {
            m_value = _v & Maximum() << Offset;
            return *this;
        }

        inline bool operator!() const
        {
            return IsZero();
        }

        inline BitFieldMember& operator+=(TrueType _v)
        {
            KE_ASSERT(TrueType(*this) + _v <= Maximum());
            m_value += _v << Offset;
            return *this;
        }

        inline BitFieldMember& operator-=(TrueType _v)
        {
            KE_ASSERT(TrueType(*this) >= _v);
            m_value -= _v << Offset;
            return *this;
        }

        inline BitFieldMember& operator++() { return *this += 1; }
        inline BitFieldMember operator++(int)
        {
            BitFieldMember v(*this);
            operator++();
            return v;
        }

        inline BitFieldMember& operator--() { return *this -= 1; }
        inline BitFieldMember operator--(int)
        {
            BitFieldMember v(*this);
            operator--();
            return v;
        }
    };

    template<class EnumType>
    inline constexpr bool EnumHasAny(const EnumType _source, const EnumType _flags)
    {
        using UnderlyingType = std::underlying_type_t<EnumType>;
        return static_cast<UnderlyingType>(_source & _flags) != 0;
    }

    template<class EnumType>
    inline constexpr bool EnumHasAll(const EnumType _source, const EnumType _flags)
    {
        using UnderlyingType = std::underlying_type_t<EnumType>;
        return static_cast<UnderlyingType>(_source & _flags) == static_cast<UnderlyingType>(_flags);
    }

    template<class T> requires std::is_integral_v<T>
    constexpr T BitfieldInsert(T _target, T _value, u8 _size, u8 _offset = 0)
    {
        return _target | (_value << _offset);
    }

    template<class T> requires std::is_integral_v<T>
    constexpr T BitfieldInsertSafe(T _target, T _value, u8 _size, u8 _offset = 0)
    {
        const T bitMask = BitMask<T>(_size) << _offset;
        return (_target & ~bitMask) | ((_value << _offset) & bitMask);
    }

    template<class T> requires std::is_integral_v<T>
    constexpr T BitfieldExtract(T _value, u8 _size, u8 _offset = 0)
    {
        return (_value >> _offset) & BitMask<T>(_size);
    }


    u8 GetMostSignificantBit(u64 _value);
    u8 GetLeastSignificantBit(u64 _value);
}