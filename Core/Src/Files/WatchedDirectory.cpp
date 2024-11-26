/**
 * @file
 * @author Max Godefroy
 * @date 21/11/2022.
 */

#include "WatchedDirectory.hpp"

#include "EASTL/hash_set.h"
#include "EASTL/sort.h"
#include "FileSystemHelper.hpp"
#include <filesystem>


namespace KryneEngine
{
    WatchedDirectory::WatchedDirectory(const eastl::string_view &_dirPath,
                                       moodycamel::ConcurrentQueue<FsChange> &_changesQueue,
                                       bool _recursive)
        : m_dirPath(_dirPath)
        , m_changesQueue(_changesQueue)
        , m_changesQueueProducerToken(m_changesQueue)
        , m_recursiveDir(_recursive)
    {
        _Browse(false);
    }

    void WatchedDirectory::_Browse(bool _notify)
    {
        namespace fs = std::filesystem;

        eastl::vector<bool> foundCurrent;
        foundCurrent.resize(m_files.size(), false);

        // Iterate over all the file of the directory.
        // Enqueue the new files, notify for files that were modified, and list found files currently in the list.
        {
            const auto iterationFunctor = [&](const fs::directory_entry &_entry)
            {
                if (_entry.is_directory())
                {
                    return;
                }

                bool notifyChange = false;

                FsChange change{StringHash(_entry.path().string().c_str())};
                const auto &pathHash = change.m_path;

                const u64 lastWriteTime = FileSystemHelper::SystemTimeToMillisecondsFromEpoch(_entry.last_write_time());

                const auto it = m_files.find(pathHash);
                if (it == m_files.end())
                {
                    notifyChange = true;
                    m_files.emplace_back_unsorted(pathHash, FileInfo { lastWriteTime });
                }
                else
                {
                    const u64 index = it - m_files.begin();
                    foundCurrent[index] = false;


                    if (lastWriteTime > it->second.m_lastWriteTime)
                    {
                        notifyChange = true;
                    }
                }

                if (notifyChange && _notify)
                {
                    m_changesQueue.enqueue(m_changesQueueProducerToken, eastl::move(change));
                }
            };

            if (m_recursiveDir)
            {
                for (const auto &entry: fs::recursive_directory_iterator(m_dirPath.c_str()))
                {
                    iterationFunctor(entry);
                }
            }
            else
            {
                for (const auto &entry: fs::directory_iterator(m_dirPath.c_str()))
                {
                    iterationFunctor(entry);
                }
            }
        }

        // Clean up deleted files
        // If they weren't found during the browsing, they are considered deleted, and their ref will be removed.
        // We do an unsorted erase for speed. There will be a new re-sorting later.
        {
            for (u64 i = 0; i < foundCurrent.size(); i++)
            {
                // We erase the elements in reverse order to ensure data validity.
                // This way, elements that are slated to be erased won't change index during the process.
                const u64 index = foundCurrent.size() - 1 - i;

                if (!foundCurrent[index])
                {
                    m_files.erase_unsorted(m_files.begin() + index);
                }
            }
        }

        // Re-sort map afterwards to keep correct behaviour.
        // Use explicit key compare functor, otherwise the default compare tries to also compare the values.
        eastl::sort(m_files.begin(), m_files.end(), [](const auto& _a, const auto& _b) {
            return _a.first < _b.first;
        });
    }

    void WatchedDirectory::Update()
    {
        _Browse();
    }
} // KryneEngine