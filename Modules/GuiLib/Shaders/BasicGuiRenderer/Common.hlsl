/**
* @file
 * @author Max Godefroy
 * @date 08/12/2025.
 */

#pragma once

#include "Platform.hlsl"

struct ViewportConstants
{
    float4x4 ndcProjectionMatrix;
    float2 viewportSize;
};

vkBinding(0, 0) ConstantBuffer<ViewportConstants> viewportConstants: register(b0, space0);

vkBinding(0, 1) Texture2D textures[32]: register(t0, space1);
vkBinding(1, 1) SamplerState samplers[8]: register(s0, space1);

static const float2 vertices[6] = {
    { -1.0f, -1.0f },
    { -1.0f,  1.0f },
    {  1.0f, -1.0f },
    {  1.0f,  1.0f },
    {  1.0f, -1.0f },
    { -1.0f,  1.0f },
};

struct VsInput
{
    vkLocation(0) uint2 packedRect: POSITION;
    vkLocation(1) uint packedColor: COLOR;
    vkLocation(2) uint4 packedData: TEXCOORD;
    uint vertexId : SV_VertexID;
};