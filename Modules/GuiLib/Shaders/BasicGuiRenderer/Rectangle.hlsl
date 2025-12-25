/**
* @file
 * @author Max Godefroy
 * @date 08/12/2025.
 */

#include "Common.hlsl"
#include "Math/ColorConversion.hlsl"
#include "Math/CoordinateTransforms.hlsl"

struct VsOutput
{
    float4 position : SV_POSITION;
    float2 pixelPosition : POSITION0;
    nointerpolation float cornerRadius : TEXCOORD0;
    nointerpolation uint backgroundColor : COLOR;
    nointerpolation float2 rectHalfSize: POSITION1;
};

using FsInput = VsOutput;

[shader("vertex")]
VsOutput RectangleVs(VsInput _input)
{
    VsOutput output;

    const float2 rectCenter = unpackHalf2x16ToFloat(_input.packedRect.x);
    const float2 rectHalfSize = unpackHalf2x16ToFloat(_input.packedRect.y);

    output.backgroundColor = _input.packedData.z;
    output.cornerRadius = unpackHalf2x16ToFloat(_input.packedData.x).x;
    output.rectHalfSize = rectHalfSize;

    const float2 baseVertexPos = vertices[_input.vertexId];
    output.pixelPosition = baseVertexPos * rectHalfSize;
    const float2 ndcPosition = UvToNdc((output.pixelPosition + rectCenter) / viewportConstants.viewportSize);
    output.position = mul(float4(ndcPosition, 0.f, 1.f), viewportConstants.ndcProjectionMatrix);
    return output;
}

// Rounded box SDF calculations based on https://www.shadertoy.com/view/4cG3R1
float SDRoundedBox(float2 _position, float2 _halfSize, float _cornerRadius)
{
    const float2 q = abs(_position) - _halfSize + _cornerRadius;

    if (min(q.x, q.y) < 0.f)
        return max(q.x, q.y) - _cornerRadius;

    const float2 uv = float2(abs(q.x - q.y), q.x + q.y - _cornerRadius) / _cornerRadius;
    return (length(uv - float2(0, -1)) - sqrt(2.f)) * _cornerRadius * sqrt(0.5f);
}

[shader("pixel")]
float4 RectangleFs(FsInput _input): SV_TARGET0
{
    const float d = SDRoundedBox(_input.pixelPosition, _input.rectHalfSize, _input.cornerRadius);

    const float2 dPixelPos = ddx(_input.pixelPosition);
    const float scale = length(dPixelPos) * 0.5;

    const float4 color = SrgbToLinear(unpackUnorm4x8ToFloat(_input.backgroundColor));

    return lerp(color, float4(color.rgb, 0.f), saturate(d / scale + 0.5f));
}