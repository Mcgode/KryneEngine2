/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#pragma once

#include "KryneEngine/Core/Platform/Windows.h"

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using Microsoft::WRL::ComPtr;

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

// D3D12 extension library.
#include <directx/d3dx12.h>

// Kryne engine includes
#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Graphics/Common/GraphicsCommon.hpp"
#include "KryneEngine/Core/Memory/GenerationalPool.hpp"
#include "KryneEngine/Core/Profiling/TracyHeader.hpp"