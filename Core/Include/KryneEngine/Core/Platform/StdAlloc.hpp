/**
 * @file
 * @author Max Godefroy
 * @date 23/02/2025.
 */

#pragma once

namespace KryneEngine::StdAlloc
{
    void* Malloc(size_t _size);

    void* MemAlign(size_t _size, size_t _alignment);

    void Free(void* _ptr);
}