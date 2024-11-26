/**
 * @file
 * @author Max Godefroy
 * @date 10/10/2022.
 */

#pragma once

#include "EASTL/span.h"
#include <Files/WatchedDirectory.hpp>

namespace KryneEngine
{
    class FileWatcher
    {
    public:
        FileWatcher(const eastl::span<eastl::string_view> &_recursiveDirectoryPaths,
                    const eastl::span<eastl::string_view> &_nonRecursiveDirectoryPaths);

        ~FileWatcher();

    private:
        eastl::vector<WatchedDirectory> m_watchedDirectories;
        std::thread m_watcherThread;

        moodycamel::ConcurrentQueue<WatchedDirectory::FsChange> m_changesQueue;
        volatile bool m_shouldStop = false;

    };
} // KryneEngine