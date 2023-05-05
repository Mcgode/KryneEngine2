/**
 * @file
 * @author Max Godefroy
 * @date 10/03/2023.
 */

#include "IoQueryManager.hpp"
#include "FileSystemHelper.hpp"
#include <Threads/FibersManager.hpp>

namespace KryneEngine
{
    IoQueryManager::IoQueryManager(FibersManager *_fibersManager)
    {
        _fibersManager->m_ioManager = this;

        m_thread = std::thread([this, _fibersManager]() {
            while (!m_shouldStop)
            {
                _ProcessIoQueries(_fibersManager);

                std::unique_lock<std::mutex> lock(m_waitMutex);
                m_waitConditionVariable.wait(lock); // Allow spurious wakeup
            }
        });
    }

    IoQueryManager::~IoQueryManager()
    {
        m_shouldStop = true;
        m_waitConditionVariable.notify_one();
        m_thread.join();
    }

    void IoQueryManager::MakeQueryAsync(IoQueryManager::Query *_query)
    {
        m_queriesQueue.enqueue(_query);
        m_waitConditionVariable.notify_one();
    }

    void IoQueryManager::MakeQuerySync(IoQueryManager::Query *_query)
    {
        _HandleQuery(_query, nullptr);
    }

    void IoQueryManager::_ProcessIoQueries(FibersManager *_fibersManager)
    {
        Query* query = nullptr;
        while(m_queriesQueue.try_dequeue(query))
        {
            _HandleQuery(query, _fibersManager);
        }
    }

    void IoQueryManager::_HandleQuery(IoQueryManager::Query *_query, FibersManager *_fibersManager)
    {
        const auto offset = _query->m_offset;
        s64 fileSize = -1;

        if (_query->m_file == nullptr)
        {
            VERIFY_OR_RETURN_VOID(_query->m_path != nullptr);

            IF_NOT_VERIFY_MSG(FileSystemHelper::Exists(_query->m_path), "No such file")
            {
                return;
            }

            auto error = fopen_s(&_query->m_file, _query->m_path, _query->m_destroyOnOpen ? "wb+" : "rb+");
            IF_NOT_VERIFY_MSG(error == 0, "Error while opening file")
            {
                return;
            }

            fseek(_query->m_file, 0, SEEK_END);
            fileSize = ftell(_query->m_file);
            _query->m_fileSize = fileSize;
        }

        // Read/write data
        if (_query->m_size > 0)
        {
            fseek(_query->m_file, offset, SEEK_SET);

            if (_query->m_type == Query::Type::Read)
            {
                const u64 readSize = fileSize < 0 ? _query->m_size : eastl::min<u64>(_query->m_size, fileSize);

                if (_query->m_data != nullptr && Verify(fileSize >= 0))
                {
                    _query->m_data = new u8[readSize];
                }

                _query->m_size = fread(_query->m_data, sizeof(u8), readSize, _query->m_file);
            }
            else
            {
                if (!Verify(_query->m_data != nullptr))
                {
                    return;
                }

                _query->m_size = fwrite(_query->m_data, sizeof(u8), _query->m_size, _query->m_file);
            }
        }

        // Close file if requested.
        if (_query->m_closeFile)
        {
            fclose(_query->m_file);
            _query->m_file = nullptr;
        }

        // Update sync counter if provided.
        if (_query->m_syncCounterId != kInvalidSyncCounterId && Verify(_fibersManager != nullptr))
        {
            _fibersManager->m_syncCounterPool.DecrementCounterValue(_query->m_syncCounterId);
        }

        // Delete query from heap.
        // This can happen if a thread wants to do a send-and-forget query.
        //   Ex: A fiber thread wants to close a file, but won't wait for the operation (too long and costly for such a simple op).
        if (_query->m_deleteQuery)
        {
            delete _query;
        }
    }
} // KryneEngine