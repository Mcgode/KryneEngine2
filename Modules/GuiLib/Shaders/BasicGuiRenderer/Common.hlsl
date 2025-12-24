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
    vkLocation(1) uint4 packedData: TEXCOORD;
    uint vertexId : SV_VertexID;
};