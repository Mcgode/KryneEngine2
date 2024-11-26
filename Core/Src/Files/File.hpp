/**
 * @file
 * @author Max Godefroy
 * @date 10/03/2023.
 */

#pragma once

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Memory/RangeMapping.hpp"

namespace KryneEngine
{
    /// @warning
    /// The class is not thread-safe.
    /// You should always only do operations in a single thread, or implement external synchronization.
    class File
    {
    public:
        explicit File(const eastl::string_view& _path);

        virtual ~File();

        u64 Open(bool _erasePreviousContent = false);
        void Close(bool _blocking = false);

        const MemoryRangeMapping& Read(u64 _size = UINT64_MAX, u64 _offset = 0);

        /// @note If file was not opened yet, it will be in write mode (i.e. it will erase previous content).
        void Write(const MemoryRangeMapping& _mappedData, bool _closeAfter = false);

        bool IsIdentical(u64 _bufferSize, u8* _buffer);
        bool WriteIfNotIdentical(u64 _bufferSize, u8* _buffer, bool _closeAfterWrite = true);
        static bool WriteIfNotIdentical(const eastl::string_view& _path, u64 _bufferSize, u8* _buffer);

    private:
        eastl::string m_path;
        FILE* m_file = nullptr;
        u64 m_fileSize = UINT64_MAX;

        MemoryRangeMapping m_fileReadMapping {};
        u64 m_allocatedMemorySize = 0;

        void _FreeReadMapping(bool _resetIndices = false);
        void _CloseFile();
    };
} // KryneEngine