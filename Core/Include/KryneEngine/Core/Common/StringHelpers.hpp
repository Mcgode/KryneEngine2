/**
 * @file
 * @author Max Godefroy
 * @date 02/04/2022.
 */

#pragma once

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <string>
#include "KryneEngine/Core/Common/Types.hpp"

namespace KryneEngine
{
    struct StringHash
    {
        explicit StringHash(u64 _value)
            : m_hash(_value)
        {}

        explicit StringHash(const eastl::string_view& _string)
            : m_string(_string)
            , m_hash(Hash64(_string))
        {}

        StringHash(const StringHash& _other) = default;
        StringHash(StringHash&& _other) noexcept
            : m_hash(_other.m_hash)
            , m_string(eastl::move(_other.m_string))
        {
        }

        u64 m_hash;
        eastl::string m_string {};


        StringHash& operator=(const StringHash& _other) = default;
        StringHash& operator=(StringHash&& _other) = default;

        bool operator==(const StringHash &rhs) const
        {
            return m_hash == rhs.m_hash;
        }

        bool operator<(const StringHash &rhs) const
        {
            return m_hash < rhs.m_hash;
        }

        static u64 Hash64(const eastl::string_view& _string)
        {
            return Murmur2Hash64(_string.data(), _string.size());
        }

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

        static u64 Fnv1AHash64(const void* _data, const u64 _size)
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
        static u64 Murmur2Hash64(const void* _data, const u64 _size)
        {
            u64 hash = kMurmurSeed ^ (_size * kMurmurPrime);

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
    };

    namespace StringHelpers
    {
        template <class Container, bool Reserve = true>
        eastl::vector<const char*> RetrieveStringPointerContainer(const Container& _container)
        {
            static_assert(
                    std::is_same<typename Container::value_type, eastl::string>::value ||
                    std::is_same<typename Container::value_type, std::string>::value,
                    "Container value type should be a string"
            );
            eastl::vector<const char*> result;

            if (Reserve)
            {
                result.reserve(_container.size());
            }

            for (const auto& str: _container)
            {
                result.push_back(str.c_str());
            }

            return result;
        }
    }
}

namespace eastl
{
    template <>
    struct hash<KryneEngine::StringHash>
    {
        size_t operator()(const KryneEngine::StringHash& _val) const { return static_cast<size_t>(_val.m_hash); }
    };
}