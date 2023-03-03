/**
 * @file
 * @author Max Godefroy
 * @date 25/11/2022.
 */

#include "FileSystemHelper.hpp"

#include <filesystem>

namespace KryneEngine::FileSystemHelper
{
    bool IsDirectory(const eastl::string_view &_path)
    {
        return std::filesystem::is_directory(_path.data());
    }

    bool Exists(const eastl::string_view &_path)
    {
        return std::filesystem::exists(_path.data());
    }

    u64 SystemTimeToMillisecondsFromEpoch(const std::chrono::time_point<std::filesystem::_File_time_clock> &_timePoint)
    {
        const auto duration = _timePoint.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

    u64 GetLastWriteTime(const eastl::string_view &_path)
    {
        return SystemTimeToMillisecondsFromEpoch(std::filesystem::last_write_time(_path.data()));
    }
}