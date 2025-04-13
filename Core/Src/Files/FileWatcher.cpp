/**
 * @file
 * @author Max Godefroy
 * @date 10/10/2022.
 */

#include "Files/FileWatcher.hpp"

#if defined(_WIN32)
#	include <KryneEngine/Core/Platform/Windows.h>
#endif

#if defined(__APPLE__)
#   include <sys/event.h>
#   include <sys/fcntl.h>
#endif


#include "KryneEngine/Core/Common/Assert.hpp"
#include "KryneEngine/Core/Memory/DynamicArray.hpp"

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
            if (m_watchedDirectories.empty())
            {
                return;
            }

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
#elif (__APPLE__)
            const s32 kq = kqueue();
            IF_NOT_VERIFY_MSG(kq != -1, "Unable to create kqueue for file watcher")
            {
                return;
            }

            const DynamicArray<s32> dirFDs(m_watchedDirectories.size());

            for (u32 i = 0; i < m_watchedDirectories.size(); i++)
            {
                const auto& directory = m_watchedDirectories[i];
                dirFDs[i] = open(directory.GetPath().c_str(), O_RDONLY);
                IF_NOT_VERIFY_MSG(dirFDs[i] != -1, "Unable to open dir descriptor for '%s'", directory.GetPath().c_str())
                {
                    return;
                }

                struct kevent event = {};
                EV_SET(
                    &event,
                    dirFDs[i],
                    EVFILT_VNODE,
                    EV_ADD | EV_ENABLE | EV_CLEAR,
                    NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_DELETE,
                    0,
                    reinterpret_cast<void*>(i));

                if (kevent(kq, &event, 1, nullptr, 0, nullptr) == -1)
                {
                    KE_ERROR("Failed to add directory to file watcher: %s", directory.GetPath().c_str());
                    close(dirFDs[i]);
                    dirFDs[i] = -1;
                }
            }

            do
            {
                struct kevent events[16];
                constexpr timespec timeout = { .tv_sec = 1, .tv_nsec = 0 };
                const s32 waitResult = kevent(kq, nullptr, 0, events, 16, &timeout);

                IF_NOT_VERIFY_MSG(waitResult != -1, "Unable to get kevent for file watcher")
                {
                    return;
                }

                for (s32 i = 0; i < waitResult; i++)
                {
                    if (events[i].filter == EVFILT_VNODE)
                    {
                        const u32 index = reinterpret_cast<intptr_t>(events[i].udata);
                        m_watchedDirectories[index].Update();
                    }
                }

                for (const s32 dirFD: dirFDs)
                {
                    if (dirFD != -1)
                    {
                        close(dirFD);
                    }
                }
                close(kq);
            }
            while (!m_shouldStop);
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