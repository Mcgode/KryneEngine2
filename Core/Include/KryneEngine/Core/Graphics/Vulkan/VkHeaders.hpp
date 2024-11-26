/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#pragma once

#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Common/Arrays.hpp"
#include "KryneEngine/Core/Common/Assert.hpp"
#include "KryneEngine/Core/Graphics/Common/GraphicsCommon.hpp"
#include "KryneEngine/Core/Memory/GenerationalPool.hpp"
#include "KryneEngine/Core/Profiling/TracyHeader.hpp"

#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>