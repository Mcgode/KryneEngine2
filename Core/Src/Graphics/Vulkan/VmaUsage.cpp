/**
 * @file
 * @author Max Godefroy
 * @date 01/08/2024.
 */

#include "KryneEngine/Core/Common/Assert.hpp"

#define VMA_IMPLEMENTATION
#define VMA_ASSERT(expr) KE_ASSERT_FATAL(expr)
#define VMA_ASSERT_LEAK(expr) KE_ASSERT(expr)

#include "vk_mem_alloc.h"
