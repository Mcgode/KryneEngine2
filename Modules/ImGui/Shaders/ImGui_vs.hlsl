struct PushConstants
{
    float2 scale;
    float2 translate;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> pushConstants: register(b0);

struct VsInput 
{
    [[vk::location(0)]] float2 position: POSITION0;
    [[vk::location(1)]] float4 color: COLOR0;
    [[vk::location(2)]] float2 uv: TEXCOORD0;
};

struct VsOutput
{
    [[vk::location(0)]] float4 color: COLOR0;
    [[vk::location(1)]] float2 uv: TEXCOORD0;
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

