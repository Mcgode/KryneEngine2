#include "Platform.hlsl"

struct PushConstants
{
    float2 scale;
    float2 translate;
};

vkPushConstant
ConstantBuffer<PushConstants> pushConstants: register(b0);

struct VsInput 
{
    vkLocation(0) float2 position: POSITION0;
    vkLocation(1) float4 color: COLOR0;
    vkLocation(2) float2 uv: TEXCOORD0;
};

struct VsOutput
{
    vkLocation(0) float4 color: COLOR0;
    vkLocation(1) float2 uv: TEXCOORD0;
    float4 position: SV_POSITION;
};

VsOutput MainVS(in VsInput input)
{
    VsOutput output;
    output.position = float4(input.position * pushConstants.scale + pushConstants.translate, 0.f, 1);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

typedef VsOutput PsInput;

