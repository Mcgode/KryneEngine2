/**
 * @file
 * @author Max Godefroy
 * @date 25/11/2022.
 */

#pragma once

#include <Common/Types.hpp>
#include <chrono>

namespace KryneEngine::FileSystemHelper
{
    [[nodiscard]] bool Exists(const eastl::string_view& _path);

    [[nodiscard]] bool IsDirectory(const eastl::string_view& _path);

    [[nodiscard]] u64 SystemTimeToMillisecondsFromEpoch(const std::chrono::time_point<std::filesystem::_File_time_clock>& _timePoint);

    [[nodiscard]] u64 GetLastWriteTime(const eastl::string_view& _path);
}
