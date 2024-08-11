#include "Platform.hlsl"

struct PsInput
{
    vkLocation(0) float4 color: COLOR0;
    vkLocation(1) float2 uv: TEXCOORD0;
};

struct PsOutput
{
    vkLocation(0) float4 color: SV_Target0;
};

vkBinding(0, 0) SamplerState sampler0: register(s0);
vkBinding(1, 0) Texture2D texture0: register(t0);

PsOutput MainPS(in PsInput input)
{
    PsOutput output;

    output.color = input.color * texture0.Sample(sampler0, input.uv);

    return output;
};