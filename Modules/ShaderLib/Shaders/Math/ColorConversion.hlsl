/**
* @file
 * @author Max Godefroy
 * @date 14/12/2025.
 */

// Based on https://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
float3 LinearToSrgb(const in float3 _lrgb)
{
    const float3 s1 = sqrt(_lrgb);
    const float3 s2 = sqrt(s1);
    const float3 s3 = sqrt(s2);

    return 0.585122381 * s1 + 0.783140355 * s2 - 0.368262736 * s3;
}

// Based on https://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
float3 SrgbtoLinear(const in float3 _srgb)
{
    return _srgb * (_srgb * (_srgb * 0.305306011 + 0.682171111) + 0.012522878);
}

float4 LinearToSrgb(const in float4 _lrgba)
{
    return float4(LinearToSrgb(_lrgba.rgb), _lrgba.a);
}

float4 SrgbToLinear(const in float4 _srgba)
{
    return float4(SrgbtoLinear(_srgba.rgb), _srgba.a);
}