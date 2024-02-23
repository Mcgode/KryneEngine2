/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#pragma once

#include <Common/Assert.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <comdef.h>
#include <Graphics/Common/Enums.hpp>
#include "Dx12Headers.hpp"

namespace KryneEngine
{
#define Dx12Verify(condition) KE_VERIFY(SUCCEEDED(condition))

    inline void Dx12Assert(HRESULT _hr)
    {
        if (!SUCCEEDED(_hr))
        {
            _com_error e(_hr);
            KE_FATAL(e.ErrorMessage());
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
        _pointer = nullptr;
    }

    template <class DxObject, class... Args>
    void Dx12SetName(DxObject* _object, const wchar_t* _format, Args... _args)
    {
        eastl::wstring name;
        name.sprintf(_format, _args...);
        name = L"[App] " + name;
        Dx12Assert(_object->SetPrivateData(WKPDID_D3DDebugObjectNameW, name.size() * sizeof(wchar_t), name.c_str()));
    }

    namespace Dx12Converters
    {
        [[nodiscard]] inline D3D_FEATURE_LEVEL GetFeatureLevel(const GraphicsCommon::ApplicationInfo& _appInfo)
        {
        	KE_ASSERT(_appInfo.IsDirectX12Api());

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

        constexpr inline DXGI_FORMAT ToDx12Format(TextureFormat _format)
        {
            DXGI_FORMAT format;

#define MAP(commonFormat, dx12Format) case TextureFormat::commonFormat: format = dx12Format; break

            switch (_format)
            {
                MAP(R8_UNorm, DXGI_FORMAT_R8_UNORM);
                MAP(RG8_UNorm, DXGI_FORMAT_R8G8_UNORM);
                MAP(RGB8_UNorm, DXGI_FORMAT_R8G8B8A8_UNORM);
                MAP(RGBA8_UNorm, DXGI_FORMAT_R8G8B8A8_UNORM);

                MAP(RGB8_sRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
                MAP(RGBA8_sRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

                MAP(BGRA8_UNorm, DXGI_FORMAT_B8G8R8A8_UNORM);
                MAP(BGRA8_sRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

                MAP(R8_SNorm, DXGI_FORMAT_R8_SNORM);
                MAP(RG8_SNorm, DXGI_FORMAT_R8G8_SNORM);
                MAP(RGB8_SNorm, DXGI_FORMAT_R8G8B8A8_SNORM);
                MAP(RGBA8_SNorm, DXGI_FORMAT_R8G8B8A8_SNORM);

                MAP(D16, DXGI_FORMAT_D16_UNORM);
                MAP(D24, DXGI_FORMAT_D24_UNORM_S8_UINT);
                MAP(D32F, DXGI_FORMAT_D32_FLOAT);
                MAP(D24S8, DXGI_FORMAT_D24_UNORM_S8_UINT);
                MAP(D32FS8, DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
                default:
                    KE_ASSERT_MSG(_format != TextureFormat::NoFormat, "Unknown format");
                    format = DXGI_FORMAT_UNKNOWN;
            }

#undef MAP

            return format;
        }

        constexpr inline TextureFormat FromDx12Format(DXGI_FORMAT _format)
        {
            TextureFormat format;

#define MAP(commonFormat, dx12Format) case dx12Format: format = TextureFormat::commonFormat; break

            switch (_format)
            {
                MAP(R8_UNorm, DXGI_FORMAT_R8_UNORM);
                MAP(RG8_UNorm, DXGI_FORMAT_R8G8_UNORM);
                // MAP(RGB8_UNorm, DXGI_FORMAT_R8G8B8A8_UNORM);
                MAP(RGBA8_UNorm, DXGI_FORMAT_R8G8B8A8_UNORM);

                // MAP(RGB8_sRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
                MAP(RGBA8_sRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

                MAP(BGRA8_UNorm, DXGI_FORMAT_B8G8R8A8_UNORM);
                MAP(BGRA8_sRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

                MAP(R8_SNorm, DXGI_FORMAT_R8_SNORM);
                MAP(RG8_SNorm, DXGI_FORMAT_R8G8_SNORM);
                // MAP(RGB8_SNorm, DXGI_FORMAT_R8G8B8A8_SNORM);
                MAP(RGBA8_SNorm, DXGI_FORMAT_R8G8B8A8_SNORM);

                MAP(D16, DXGI_FORMAT_D16_UNORM);
                // MAP(D24, DXGI_FORMAT_D24_UNORM_S8_UINT);
                MAP(D32F, DXGI_FORMAT_D32_FLOAT);
                MAP(D24S8, DXGI_FORMAT_D24_UNORM_S8_UINT);
                MAP(D32FS8, DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
                default:
                    KE_ASSERT_MSG(_format != DXGI_FORMAT_UNKNOWN, "Unknown format");
                    format = TextureFormat::NoFormat;
            }

#undef MAP

            return format;
        }
    }

    void DebugLayerMessageCallback(
            D3D12_MESSAGE_CATEGORY _category,
            D3D12_MESSAGE_SEVERITY _severity,
            D3D12_MESSAGE_ID _id,
            LPCSTR _description,
            void* _context);
}