/**
 * @file
 * @author Max Godefroy
 * @date 20/11/2022.
 */

#pragma once

#include <Common/KETypes.hpp>
#include <Common/Assert.hpp>

namespace KryneEngine::BitUtils
{
    template<class T>
    inline constexpr T BitMask(u8 _size)
    {
        return ((1 << _size) - 1);
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
            Assert(TrueType(*this) + _v <= Maximum());
            m_value += _v << Offset;
            return *this;
        }

        inline BitFieldMember& operator-=(TrueType _v)
        {
            Assert(TrueType(*this) >= _v);
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
}