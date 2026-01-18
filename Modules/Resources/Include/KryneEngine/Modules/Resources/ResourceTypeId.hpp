/**
 * @file
 * @author Max Godefroy
 * @date 16/01/2026.
 */

#pragma once


#include <KryneEngine/Core/Common/Types.hpp>

namespace KryneEngine::Modules::Resources
{
    using ResourceTypeId = u64;

    constexpr ResourceTypeId GenerateResourceTypeId(const char* _name, size_t _size)
    {
        u64 value = 0;
        switch (_size > 8 ? 8 : _size)
        {
        case 8: value |= static_cast<u64>(_name[7]) << 56;
        case 7: value |= static_cast<u64>(_name[6]) << 48;
        case 6: value |= static_cast<u64>(_name[5]) << 40;
        case 5: value |= static_cast<u64>(_name[4]) << 32;
        case 4: value |= static_cast<u64>(_name[3]) << 24;
        case 3: value |= static_cast<u64>(_name[2]) << 16;
        case 2: value |= static_cast<u64>(_name[1]) << 8;
        case 1: value |= static_cast<u64>(_name[0]);
        default: break;
        }

        // Based on Murmur2 hashing, see Math/Hashing.cpp
        constexpr u64 kMurmurSeed = 0x9E37'79B9'7F4A'7C15ull;
        constexpr u64 kMurmurPrime = 14'313'749'767'032'793'493ull;

        return kMurmurSeed ^ (value * kMurmurPrime);
    }

    template <size_t N>
    constexpr ResourceTypeId GenerateResourceTypeId(const char (&_name)[N])
    {
        constexpr size_t len = (N == 0 ? 0 : N - 1); // N is >=1 for string literal
        return GenerateResourceTypeId(_name, len);
    }
}