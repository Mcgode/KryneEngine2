/**
 * @file
 * @author Max Godefroy
 * @date 02/04/2022.
 */

#pragma once

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <string>

namespace KryneEngine::StringHelpers
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

        u64 m_hash;
        eastl::string m_string {};

        // Based on FNV hash
        // http://isthe.com/chongo/tech/comp/fnv/
        static u64 Hash64(const eastl::string_view& _string)
        {
            u64 hash = 14695981039346656037u;
            for (char c: _string)
            {
                hash = (hash * 1099511628211u) ^ static_cast<u64>(c);
            }
            return hash;
        }
    };

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