/**
 * @file
 * @author Max Godefroy
 * @date 25/11/2022.
 */

#pragma once

#include <filesystem>
#include "KryneEngine/Core/Common/Types.hpp"

namespace KryneEngine::FileSystemHelper
{
    [[nodiscard]] bool Exists(const eastl::string_view& _path);

    [[nodiscard]] bool IsDirectory(const eastl::string_view& _path);

    [[nodiscard]] u64 SystemTimeToMillisecondsFromEpoch(const std::filesystem::file_time_type& _timePoint);

    [[nodiscard]] u64 GetLastWriteTime(const eastl::string_view& _path);
}
