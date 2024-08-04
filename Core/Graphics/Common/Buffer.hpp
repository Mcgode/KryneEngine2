/**
 * @file
 * @author Max Godefroy
 * @date 03/08/2024.
 */

#pragma once

#include <Graphics/Common/Enums.hpp>
#include <Graphics/Common/Handles.hpp>

namespace KryneEngine
{
    struct BufferDesc
    {
        u64 m_size;

#if !defined(KE_FINAL)
        eastl::string m_debugName;
#endif
    };

    struct BufferCreateDesc
    {
        BufferDesc m_desc;
        MemoryUsage m_usage = MemoryUsage::Undefined_UsageType;
    };

    struct BufferMapping
    {
        void* m_ptr = nullptr;
        u64 m_size;
        u64 m_offset;
        BufferHandle m_buffer;
        bool m_pureWrite;

        explicit BufferMapping(BufferHandle _buffer, u64 _size = ~0, u64 _offset = 0, bool _pureWrite = true)
            : m_buffer(_buffer)
            , m_size(_size)
            , m_offset(_offset)
            , m_pureWrite(_pureWrite)
        {}
    };

    struct BufferCopyParameters
    {
        u64 m_copySize;

        BufferHandle m_bufferSrc;
        BufferHandle m_bufferDst;

        u64 m_offsetSrc = 0;
        u64 m_offsetDst = 0;
    };
}