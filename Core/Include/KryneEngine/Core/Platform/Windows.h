/**
 * @file
 * @author Max Godefroy
 * @date 11/09/2024.
 */

#pragma once

#if !defined(KE_FULL_WINDOWS) && !defined(WIN32_LEAN_AND_MEAN)
#	define WIN32_LEAN_AND_MEAN
#endif

#if !defined(NOMINMAX)
#   define NOMINMAX
#endif

#include <windows.h>