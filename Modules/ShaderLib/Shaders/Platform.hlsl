
#define vkPushConstant [[vk::push_constant]]
#define vkLocation(index) [[vk::location(index)]]
#define vkBinding(index, set) [[vk::binding(index, set)]]

float2 unpackHalf2x16ToFloat(in uint _packedHalf2)
{
    return f16tof32(uint2(_packedHalf2 & 0xffff, _packedHalf2 >> 16));
}

float4 unpackUnorm4x8ToFloat(uint _packedUnorm)
{
    return float4(unpack_u8u32(_packedUnorm)) / 255.0f;
}