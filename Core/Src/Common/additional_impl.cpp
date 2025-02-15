/**
 * @file
 * @author Max Godefroy
 * @date 10/10/2021.
 */

#include <cstdlib>

void* __cdecl operator new[](
    const size_t _size,
    const char* /* _name */,
    int /* _flags */,
    unsigned /* _debugFlags */,
    const char* /* _file */,
    int /* _line */)
{
    return std::malloc(_size);
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
    return reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(std::aligned_alloc(_alignment, _size)) + _alignmentOffset);
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
    ::operator delete(_p);
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
    ::operator delete[](_p);
  }
}