/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#include "Platform.hlsl"
#include "FrameData.hlsl"

ConstantBuffer<FrameData> frameData;

struct VsInput
{
    float3 position: POSITION0;
    float3 normal: NORMAL0;
};

struct VsOutput
{
    float3 normal: NORMAL;
    float4 position: SV_Position;
};

VsOutput MainVs(const VsInput _input)
{
    VsOutput output;

    output.normal = mul(frameData.m_torusWorldMatrix, float4(_input.normal, 0.f)).xyz;

    output.position = mul(frameData.m_viewProjectionMatrix, mul(frameData.m_torusWorldMatrix, float4(_input.position, 1.f)));

    return output;
}

typedef VsOutput PsInput;

struct PsOutput
{
    float4 albedo: SV_TARGET0;
    float4 normal: SV_TARGET1;
};

PsOutput MainPs(PsInput _input)
{
    PsOutput output;

    output.albedo = float4(frameData.m_torusAlbedo, 0.f);
    output.normal = float4(normalize(_input.normal), 0.f);

    return output;
}
