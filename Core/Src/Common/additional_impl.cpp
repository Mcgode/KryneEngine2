/**
 * @file
 * @author Max Godefroy
 * @date 10/10/2021.
 */

#include <new>

void* __cdecl operator new[](
    const size_t _size,
    const char* /* _name */,
    int /* _flags */,
    unsigned /* _debugFlags */,
    const char* /* _file */,
    int /* _line */)
{
    return operator new(_size);
}

void* __cdecl operator new[](
    const size_t _size,
    const size_t _alignment,
    const size_t _alignmentOffset,
    const char* /* _name */,
    int /* _flags */,
    unsigned /* _debugFlags */,
    const char* /* _file */,
    int /* _line */)
{
    void* ptr = operator new(_size, std::align_val_t(_alignment));
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) + _alignmentOffset);
}

void __cdecl operator delete(
    void* _p,
    const char* /* _name */,
    int /* _flags */,
    unsigned /* _debugFlags */,
    const char* /* _file */,
    int /* _line */)
{
    if (_p != nullptr)
    {
        operator delete(_p);
    }
}

void __cdecl operator delete[](
    void* _p,
    const char* /* _name */,
    int /* _flags */,
    unsigned /* _debugFlags */,
    const char* /* _file */,
    int /* _line */)
{
    if (_p != nullptr)
    {
        operator delete[](_p);
    }
}