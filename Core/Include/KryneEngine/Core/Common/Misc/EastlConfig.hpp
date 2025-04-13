/**
 * @file
 * @author Max Godefroy
 * @date 15/02/2025.
 */

#pragma once

#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"

namespace KryneEngine::EastlConfig
{
    static KryneEngine::AllocatorInstance s_defaultAllocator;
    inline KryneEngine::AllocatorInstance* GetDefaultAllocator() { return &s_defaultAllocator; }
}

#define EASTLAllocatorType KryneEngine::AllocatorInstance
#define EASTLAllocatorDefault KryneEngine::EastlConfig::GetDefaultAllocator