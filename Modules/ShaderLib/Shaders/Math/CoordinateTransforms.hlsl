/**
 * @file
 * @author Max Godefroy
 * @date 02/04/2025.
 */

float2 ScreenSpaceToUv(const in float2 _ssCoords, const in float2 _resolution)
{
    return _ssCoords / _resolution;
}

float2 ScreenSpaceToUv(const in uint2 _pixelCoordinates, const in float2 _resolution)
{
    return ScreenSpaceToUv(float2(_pixelCoordinates) + 0.5, _resolution);
}

float2 UvToNdc(const in float2 _uvCoords)
{
    return float2(
        2.f * _uvCoords.x - 1.f,
        1.f - 2.f * _uvCoords.y
    );
}

float2 ScreenSpaceToNdc(const in float2 _ssCoords, const in float2 _resolution)
{
    return UvToNdc(ScreenSpaceToUv(_ssCoords, _resolution));
}