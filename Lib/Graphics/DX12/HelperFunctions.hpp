/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#pragma once

#include <Common/Assert.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <comdef.h>
#include "Dx12Headers.hpp"

namespace KryneEngine
{
    inline bool Dx12Verify(HRESULT _hr)
    {
        return Verify(SUCCEEDED(_hr));
    }

    inline void Dx12Assert(HRESULT _hr)
    {
        if (!SUCCEEDED(_hr))
        {
            _com_error e(_hr);
            Assert(false, e.ErrorMessage());
        }
    }

    template <class T>
    inline void SafeRelease(T& _pointer)
    {
        if (_pointer != nullptr) [[likely]]
        {
            _pointer->Release();
            _pointer = nullptr;
        }
    }

    template <class T>
    inline void SafeRelease(ComPtr<T>& _pointer)
    {
        // Com pointers do internal release on assignment, so simply reset value to nullptr
        _pointer = nullptr;
    }

    namespace Dx12Converters
    {
        [[nodiscard]] inline D3D_FEATURE_LEVEL GetFeatureLevel(const GraphicsCommon::ApplicationInfo& _appInfo)
        {
            Assert(_appInfo.IsDirectX12Api());

            switch (_appInfo.m_api)
            {
                case GraphicsCommon::Api::DirectX12_2:
                    return D3D_FEATURE_LEVEL_12_2;
                case GraphicsCommon::Api::DirectX12_1:
                    return D3D_FEATURE_LEVEL_12_1;
                default:
                    return D3D_FEATURE_LEVEL_12_0;
            }
        }
    }
}