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