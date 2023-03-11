/**
 * @file
 * @author Max Godefroy
 * @date 10/03/2023.
 */

#pragma once

#include <condition_variable>
#include <EASTL/unique_ptr.h>
#include <moodycamel/concurrentqueue.h>
#include <Common/KETypes.hpp>
#include <Threads/SyncCounterPool.hpp>

namespace KryneEngine
{
    class FibersManager;

    class IoQueryManager
    {
    public:
        explicit IoQueryManager(FibersManager* _fibersManager);

        ~IoQueryManager();

        struct Query
        {
            enum class Type: u8 {
                Read,
                Write
            };

            const char* m_path = nullptr;

            FILE* m_file = nullptr;

            u8* m_data = nullptr;

            u64 m_size = UINT64_MAX;

            union
            {
                s64 m_offset = 0;
                u64 m_fileSize;
            };

            SyncCounterId m_syncCounterId = kInvalidSyncCounterId;
            Type m_type = Type::Read;
            bool m_destroyOnOpen = false;
            bool m_closeFile = false;
            bool m_deleteQuery = false;
        };

        void MakeQueryAsync(Query* _query);
        static void MakeQuerySync(Query* _query);

    private:

        moodycamel::ConcurrentQueue<Query*> m_queriesQueue;

        volatile bool m_shouldStop = false;
        std::thread m_thread;
        std::mutex m_waitMutex;
        std::condition_variable m_waitConditionVariable;

        void _ProcessIoQueries(FibersManager *_fibersManager);

        static void _HandleQuery(Query* _query, FibersManager* _fibersManager);
    };
} // KryneEngine