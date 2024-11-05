/**
 * @file
 * @author Max Godefroy
 * @date 05/11/2024.
 */

#pragma once

#include <Graphics/Common/ShaderPipeline.hpp>

namespace KryneEngine
{
    struct RenderDynamicState
    {
        float4 m_blendFactor;

        u64 m_depthStencilHash;

        float m_depthBias;
        float m_depthBiasSlope;

        float m_depthBiasClamp;
        RasterStateDesc::FillMode m_fillMode;
        RasterStateDesc::CullMode m_cullMode;
        RasterStateDesc::Front m_front : 7;
        bool m_depthClip: 1;
        u8 stencilRefValue;
    };
}