struct PsInput
{
    [[vk::location(0)]] float4 color: COLOR0;
    [[vk::location(1)]] float2 uv: TEXCOORD0;
};

struct PsOutput
{
    [[vk::location(0)]] float4 color: SV_Target0;
};

[[vk::binding(0, 0)]] SamplerState sampler0: register(s0);
[[vk::binding(1, 0)]] Texture2D texture0: register(t0);

PsOutput MainPS(in PsInput input)
{
    PsOutput output;

    output.color = input.color * texture0.Sample(sampler0, input.uv);

    return output;
};