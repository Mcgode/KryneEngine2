/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#pragma once

#include <tracy/Tracy.hpp>

#define KE_ZoneScopedFunction(name) ZoneScopedNC(name, KE_TRACY_FUNC_COLOR)
#define KE_ZoneScoped(name) ZoneScopedNC(name, KE_TRACY_COLOR)

namespace KryneEngine
{}