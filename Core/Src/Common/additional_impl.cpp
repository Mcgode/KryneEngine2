/**
 * @file
 * @author Max Godefroy
 * @date 10/10/2021.
 */

#include <KryneEngine/Core/Common/Types.hpp>

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new KryneEngine::u8[size];
}

void* __cdecl operator new[](size_t size, size_t alignment, size_t alignment_offset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new KryneEngine::u8[size];
}

void __cdecl operator delete(void* _p, const char* _name, int _flags, unsigned int _debugFlags, const char* _file, int _line)
{
  if (_p != nullptr)
  {
    ::operator delete(_p);
  }
}

void __cdecl operator delete[](void* _p, const char* _name, int _flags, unsigned int _debugFlags, const char* _file, int _line)
{
  if (_p != nullptr)
  {
    ::operator delete[](_p);
  }
}