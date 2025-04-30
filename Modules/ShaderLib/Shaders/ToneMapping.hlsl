/**
 * @file
 * @author Max Godefroy
 * @date 30/04/2025.
 */

// Functions taken from https://mini.gmshaders.com/p/tonemaps

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
float3 Tonemap_ACES(float3 _rawColor)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (_rawColor * (a * _rawColor + b)) / (_rawColor * (c * _rawColor + d) + e);
}

// Hable 2010, "Filmic Tonemapping Operators"
float3 Tonemap_Uncharted2(float3 _rawColor)
{
    _rawColor *= 16.0;
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;

    return ((_rawColor*(A*_rawColor+C*B)+D*E)/(_rawColor*(A*_rawColor+B)+D*F))-E/F;
}

// Unreal 3, Documentation: "Color Grading"
// Adapted to be close to Tonemap_ACES, with similar range
float3 Tonemap_Unreal(float3 _rawColor)
{
    // Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    return _rawColor / (_rawColor + 0.155) * 1.019;
}

float3 Tonemap_tanh(float3 _rawColor)
{
    _rawColor = clamp(_rawColor, -40.0, 40.0);
    return (exp(_rawColor) - exp(-_rawColor)) / (exp(_rawColor) + exp(-_rawColor));
}