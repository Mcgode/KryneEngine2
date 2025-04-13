/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#pragma once

#include <EASTL/unique_ptr.h>
#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"

namespace KryneEngine
{
    template<typename T>
    using UniquePtr = eastl::unique_ptr<T, AllocatorInstanceDeleter<T>>;
}