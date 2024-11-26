/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#pragma once

#include <tracy/Tracy.hpp>

#define KE_TRACY_DEFAULT_COLOR 0x0F4B80
#define KE_TRACY_DEFAULT_FUNC_COLOR 0x1876C9

#if !defined(KE_TRACY_COLOR)
#   define KE_TRACY_COLOR KE_TRACY_DEFAULT_COLOR
#endif

#if !defined(KE_TRACY_FUNC_COLOR)
#   define KE_TRACY_FUNC_COLOR KE_TRACY_DEFAULT_FUNC_COLOR
#endif

#define KE_ZoneScopedFunction(name) ZoneScopedNC(name, KE_TRACY_FUNC_COLOR)
#define KE_ZoneScoped(name) ZoneScopedNC(name, KE_TRACY_COLOR)

namespace KryneEngine
{}