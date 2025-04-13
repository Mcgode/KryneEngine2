/**
 * @file
 * @author Max Godefroy
 * @date 05/03/2025.
 */

#include "KryneEngine/Core/Math/Hashing.hpp"

namespace KryneEngine
{
    static constexpr u64 kFnvPrime = 1'099'511'628'211u;
    static constexpr u64 kFnvOffsetBasis = 14'695'981'039'346'656'037u;

    // Based on FNV hash
    // http://isthe.com/chongo/tech/comp/fnv/
    static u64 Fnv1Hash64(const void* _data, const u64 _size)
    {
        u64 hash = kFnvOffsetBasis;
        const auto* data = static_cast<const u8*>(_data);
        for (u64 i = 0; i < _size; i++)
        {
            hash = (hash * kFnvPrime) ^ static_cast<u64>(data[i]);
        }
        return hash;
    }

    inline u64 Fnv1AHash64(const void* _data, const u64 _size)
    {
        u64 hash = kFnvOffsetBasis;
        const auto* data = static_cast<const u8*>(_data);
        for (u64 i = 0; i < _size; i++)
        {
            hash = (hash ^ static_cast<u64>(data[i])) * kFnvPrime;
        }
        return hash;
    }

    static constexpr u64 kMurmurSeed = 123'456'789u;
    static constexpr u64 kMurmurPrime = 14'313'749'767'032'793'493u;
    static constexpr u64 kMurmurShift = 47u;

    // https://github.com/abrandoned/murmur2/blob/master/MurmurHash2.c
    inline u64 Murmur2Hash64(const void* _data, const u64 _size, u64 _base)
    {
        u64 hash = _base;

        const auto* data = static_cast<const u64*>(_data);
        const auto* end = data + _size / 8;
        while (data != end)
        {
            u64 k = *data++;

            k *= kMurmurPrime;
            k ^= k >> kMurmurShift;
            k *= kMurmurPrime;

            hash ^= k;
            hash *= kMurmurPrime;
        }

        const auto* data2 = reinterpret_cast<const u8*>(data);

        switch (_size & 7)
        {
        case 7: hash ^= static_cast<u64>(data2[6]) << 48;
        case 6: hash ^= static_cast<u64>(data2[5]) << 40;
        case 5: hash ^= static_cast<u64>(data2[4]) << 32;
        case 4: hash ^= static_cast<u64>(data2[3]) << 24;
        case 3: hash ^= static_cast<u64>(data2[2]) << 16;
        case 2: hash ^= static_cast<u64>(data2[1]) << 8;
        case 1: hash ^= static_cast<u64>(data2[0]);
            hash *= kMurmurPrime;
        }

        hash ^= hash >> kMurmurShift;
        hash *= kMurmurPrime;
        hash ^= hash >> kMurmurShift;

        return hash;
    }

    inline u64 Murmur2Hash64(const void* _data, const u64 _size)
    {
        return Murmur2Hash64(_data, _size, kMurmurSeed ^ (_size * kMurmurPrime));
    }
}

namespace KryneEngine::Hashing
{
    u64 Hash64(const std::byte* _data, size_t _size)
    {
        return Murmur2Hash64(_data, _size);
    }

    u64 Hash64Append(const std::byte* _data, size_t _size, u64 _accumulatedHash)
    {
        return Murmur2Hash64(_data, _size, _accumulatedHash);
    }
}