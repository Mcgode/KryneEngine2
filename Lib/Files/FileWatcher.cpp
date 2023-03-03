/**
 * @file
 * @author Max Godefroy
 * @date 10/10/2022.
 */

#include "FileWatcher.hpp"

#if defined(_WIN32)
    #include <windows.h>
#endif

#include <Common/Assert.hpp>

namespace KryneEngine
{
    FileWatcher::FileWatcher(const eastl::span<eastl::string_view> &_recursiveDirectoryPaths,
                             const eastl::span<eastl::string_view> &_nonRecursiveDirectoryPaths)
    {
        m_watchedDirectories.reserve(_recursiveDirectoryPaths.size() + _nonRecursiveDirectoryPaths.size());

        for (const auto& dirPath: _recursiveDirectoryPaths)
        {
            m_watchedDirectories.emplace_back(dirPath, m_changesQueue, true);
        }
        for (const auto& dirPath: _nonRecursiveDirectoryPaths)
        {
            m_watchedDirectories.emplace_back(dirPath, m_changesQueue, false);
        }

        m_watcherThread = std::thread([&, this]()
        {
#if defined(_WIN32)
            const DWORD filters = FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;

            eastl::vector<HANDLE> handles;
            handles.reserve(m_watchedDirectories.size());

            for (const auto& watchedDirectory: m_watchedDirectories)
            {
                auto path = reinterpret_cast<LPCWSTR>(watchedDirectory.GetPath().c_str());
                auto& handle = handles.push_back();
                handle = FindFirstChangeNotificationW(path, true, filters);

                IF_NOT_VERIFY_MSG(handle != nullptr, "Unable to init notifier")
                {
                    return;
                }
            }

            while (!m_shouldStop)
            {
                bool hadChange = false;

                u64 offset = 0;
                u64 count = eastl::min<u64>(MAXIMUM_WAIT_OBJECTS, handles.size());

                do
                {
                    auto status = WaitForMultipleObjects(
                            count,
                            handles.data() + offset,
                            FALSE,
                            WAIT_TIMEOUT);

                    if (status < WAIT_OBJECT_0 + count)
                    {
                        hadChange = true;
                        m_watchedDirectories[status + offset].Update();
                    }

                    if (offset + count < handles.size())
                    {
                        offset += count;
                    }
                    else
                    {
                        offset = 0;
                    }
                    count = eastl::min<u64>(MAXIMUM_WAIT_OBJECTS, handles.size() - offset);

                    if (m_shouldStop)
                    {
                        break;
                    }
                }
                while (!hadChange);
            }

            for (auto handle: handles)
            {
                FindCloseChangeNotification(handle);
            }
#else
#error Unsupported
#endif
        });
    }

    FileWatcher::~FileWatcher()
    {
        m_shouldStop = true;
        m_watcherThread.join();
    }
} // KryneEngine