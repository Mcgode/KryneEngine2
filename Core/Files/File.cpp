/**
 * @file
 * @author Max Godefroy
 * @date 10/03/2023.
 */

#include "File.hpp"

#include <Threads/FibersManager.hpp>
#include <Threads/FiberThread.hpp>
#include <Files/IoQueryManager.hpp>

namespace KryneEngine
{
    namespace
    {
        template<class QueryBuilderFunc, class QueryHandlerFunc>
        inline void SendIoQuery(QueryBuilderFunc _builderFunc, QueryHandlerFunc _handlerFunc, bool _doNotWaitForResult = false)
        {
            const bool isFiberThread = FiberThread::IsFiberThread();

            if (_doNotWaitForResult && isFiberThread)
            {
                auto *query = new IoQueryManager::Query();
                _builderFunc(*query);
                query->m_deleteQuery = true;
                FibersManager::GetInstance()->GetIoQueryManager()->MakeQueryAsync(query);
            }
            else
            {
                IoQueryManager::Query query;
                _builderFunc(query);

                if (isFiberThread)
                {
                    const auto fibersManager = FibersManager::GetInstance();
                    const auto autoSyncCounter = fibersManager->AcquireAutoSyncCounter();
                    query.m_syncCounterId = autoSyncCounter.GetId();
                    fibersManager->GetIoQueryManager()->MakeQueryAsync(&query);
                    fibersManager->WaitForCounter(autoSyncCounter.GetId());
                }
                else
                {
                    IoQueryManager::MakeQuerySync(&query);
                }

                _handlerFunc(query);
            }
        }
    }

    File::File(const eastl::string_view &_path)
        :m_path(_path)
    {
    }

    File::~File()
    {
        Close();
    }

    u64 File::Open(bool _erasePreviousContent)
    {
        if (m_file == nullptr)
        {
            const auto processQuery = [this, _erasePreviousContent](IoQueryManager::Query& _query)
            {
                _query.m_path = m_path.c_str();
                _query.m_size = 0; // Only open file
                _query.m_destroyOnOpen = _erasePreviousContent;
            };

            const auto handleQueryResult = [this](const IoQueryManager::Query& _query)
            {
                m_file = _query.m_file;
                m_fileSize = _query.m_size;
            };

            SendIoQuery(processQuery, handleQueryResult);
        }

        return m_fileSize;
    }

    void File::Close(bool _blocking)
    {
        if (m_file == nullptr)
        {
            return;
        }

        const auto processQuery = [this](IoQueryManager::Query& _query)
        {
            _query.m_file = m_file;
            _query.m_size = 0; // Only close file
            _query.m_closeFile = true;
        };

        const auto handleQueryResult = [](const IoQueryManager::Query& _query)
        {};

        SendIoQuery(processQuery, handleQueryResult, !_blocking);
        _CloseFile();
    }

    const MemoryRangeMapping &File::Read(u64 _size, u64 _offset)
    {
        const auto readSize = eastl::min<u64>(m_fileSize, _size);

        if (m_fileReadMapping.m_size != readSize || m_fileReadMapping.m_offset != _offset)
        {
            if (m_allocatedMemorySize < readSize)
            {
                _FreeReadMapping();
            }

            const auto processQuery = [&](IoQueryManager::Query& _query)
            {
                _query.m_path = m_path.c_str();
                _query.m_file = m_file;
                _query.m_size = readSize;
                _query.m_offset = _offset;
                _query.m_data = m_fileReadMapping.m_buffer;
                _query.m_type = IoQueryManager::Query::Type::Read;
            };

            const auto handleQueryResult = [this, _offset](const IoQueryManager::Query& _query)
            {
                // Handle case where file was not opened.
                if (m_file == nullptr)
                {
                    m_file = _query.m_file;
                    m_fileSize = _query.m_fileSize;
                }

                m_fileReadMapping.m_size = _query.m_size;
                m_fileReadMapping.m_offset = _offset;
                m_fileReadMapping.m_buffer = _query.m_data;
            };

            SendIoQuery(processQuery, handleQueryResult);
        }

        return m_fileReadMapping;
    }

    void File::Write(const MemoryRangeMapping &_mappedData, bool _closeAfter)
    {
        KE_ASSERT_MSG(_mappedData.m_buffer != nullptr || _mappedData.m_size == 0, "No provided buffer");
        KE_ASSERT_MSG(_mappedData.m_offset == 0 || m_file != nullptr,
               "File not opened yet. Non-zero write offset will result in undefined behaviour.");

        const auto processQuery = [&](IoQueryManager::Query& _query)
        {
            _query.m_path = m_path.c_str();
            _query.m_file = m_file;
            _query.m_size = _mappedData.m_size;
            _query.m_offset = _mappedData.m_offset;
            _query.m_data = _mappedData.m_buffer;
            _query.m_type = IoQueryManager::Query::Type::Write;
            _query.m_destroyOnOpen = true;
            _query.m_closeFile = _closeAfter;
        };

        const auto handleQueryResult = [&](const IoQueryManager::Query& _query)
        {
            // Handle case where file was not previously opened.
            if (m_file == nullptr)
            {
                m_file = _query.m_file;
                m_fileSize = _query.m_size; // Erased previous content, use new written values size.
            }
        };

        SendIoQuery(processQuery, handleQueryResult);
    }

    bool File::IsIdentical(u64 _bufferSize, u8 *_buffer)
    {
        if (m_file != nullptr && m_fileSize != _bufferSize)
        {
            return false;
        }

        Read();

        if (m_fileReadMapping.m_size != _bufferSize)
        {
            return false;
        }

        return memcmp(m_fileReadMapping.m_buffer, _buffer, _bufferSize) == 0;
    }

    bool File::WriteIfNotIdentical(u64 _bufferSize, u8 *_buffer, bool _closeAfterWrite)
    {
        const auto identical = IsIdentical(_bufferSize, _buffer);
        if (!identical)
        {
            Close(); // Close first, to enforce reopen with previous content erasure.
            Write({ _bufferSize, 0, _buffer },_closeAfterWrite);
        }
        return !identical;
    }

    bool File::WriteIfNotIdentical(const eastl::string_view &_path, u64 _bufferSize, u8 *_buffer)
    {
        File file(_path);
        return file.WriteIfNotIdentical(_bufferSize, _buffer);
    }

    void File::_FreeReadMapping(bool _resetIndices)
    {
        if (m_fileReadMapping.m_buffer != nullptr)
        {
            delete m_fileReadMapping.m_buffer;
            m_fileReadMapping.m_buffer = nullptr;
            m_allocatedMemorySize = 0;
        }

        if (_resetIndices)
        {
            m_fileReadMapping.m_size = 0;
            m_fileReadMapping.m_offset = 0;
        }
    }

    void File::_CloseFile()
    {
        m_file = nullptr;
        m_fileSize = UINT64_MAX;
        _FreeReadMapping(true);
    }
} // KryneEngine