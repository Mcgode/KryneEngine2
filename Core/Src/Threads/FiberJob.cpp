/**
 * @file
 * @author Max Godefroy
 * @date 02/07/2022.
 */

#include "KryneEngine/Core/Threads/FiberJob.hpp"

namespace KryneEngine
{
    FiberJob::FiberJob()
    {}

    void FiberJob::_SetContext(u16 _contextId, FiberContext *_context)
    {
        m_contextId = _contextId;
        m_context = _context;
    }

    void FiberJob::_ResetContext()
    {
        m_contextId = kInvalidContextId;
        m_context = nullptr;
    }
} // KryneEngine