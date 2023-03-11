/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#pragma once

#include <Common/Assert.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include "Dx12Headers.hpp"

namespace KryneEngine
{
    bool Dx12Verify(HRESULT _hr)
    {
        return Verify(SUCCEEDED(_hr));
    }

    void Dx12Assert(HRESULT _hr)
    {
        Assert(SUCCEEDED(_hr));
    }

    namespace Dx12Converters
    {
        D3D_FEATURE_LEVEL GetFeatureLevel(const GraphicsCommon::ApplicationInfo& _appInfo)
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