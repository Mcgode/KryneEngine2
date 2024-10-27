/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#pragma once

#include <Common/Types.hpp>
#include <Common/Arrays.hpp>
#include <Common/Assert.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Memory/GenerationalPool.hpp>
#include <Profiling/TracyHeader.hpp>

#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>