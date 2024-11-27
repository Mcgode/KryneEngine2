/**
 * @file
 * @author Max Godefroy
 * @date 05/11/2024.
 */

#pragma once

#include "KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp"
#include "KryneEngine/Core/Graphics/Common/Buffer.hpp"

namespace KryneEngine
{
    // Default values match the default values from the Metal doc
    struct RenderDynamicState
    {
        float4 m_blendFactor { 0.f, 0.f, 0.f, 0.f };

        u64 m_depthStencilHash = 0;

        float m_depthBias = 0.f;
        float m_depthBiasSlope = 0.f;

        float m_depthBiasClamp = 0.f;
        RasterStateDesc::FillMode m_fillMode = RasterStateDesc::FillMode::Solid;
        RasterStateDesc::CullMode m_cullMode = RasterStateDesc::CullMode::None;
        RasterStateDesc::Front m_front : 7 = RasterStateDesc::Front::Clockwise;
        bool m_depthClip : 1 = true;
        u8 m_stencilRefValue = 0;
    };

    struct RenderState
    {
        RenderDynamicState m_dynamicState {};
        BufferView m_indexBufferView {};
        bool m_indexBufferIsU16 = false;
        InputAssemblyDesc::PrimitiveTopology m_topology {};
        eastl::fixed_vector<BufferView, 4> m_vertexBuffers;
    };
}