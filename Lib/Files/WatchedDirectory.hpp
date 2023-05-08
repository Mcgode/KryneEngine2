/**
 * @file
 * @author Max Godefroy
 * @date 21/11/2022.
 */

#pragma once

#include <moodycamel/concurrentqueue.h>
#include <Common/StringHelpers.hpp>
#include <filesystem>
#include <EASTL/vector_map.h>

namespace KryneEngine
{
    class WatchedDirectory
    {
    public:
        struct FsChange
        {
            StringHash m_path;
        };

        struct FileInfo
        {
            u64 m_lastWriteTime = 0;
        };

        WatchedDirectory(
                const eastl::string_view &_dirPath,
                moodycamel::ConcurrentQueue<FsChange> &_changesQueue,
                bool _recursive);

        [[nodiscard]] const eastl::string &GetPath() const
        {
            return m_dirPath;
        }

        void Update();

    private:
        eastl::string m_dirPath;
        moodycamel::ConcurrentQueue<FsChange>& m_changesQueue;
        moodycamel::ProducerToken m_changesQueueProducerToken;

        bool m_recursiveDir;
        eastl::vector_map<StringHash, FileInfo> m_files;

        void _Browse(bool _notify = true);
    };
} // KryneEngine