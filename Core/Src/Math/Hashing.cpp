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
}