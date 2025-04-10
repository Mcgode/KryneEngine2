/**
 * @file
 * @author Max Godefroy
 * @date 10/04/2025.
 */

#include "Platform.hlsl"
#include "../FrameData.hlsl"

struct Input {
    float4 screenPosition: SV_Position;
};

struct Output {
    float4 color: SV_Target0;
};

Output SkyMain(Input _input)
{
    Output output;

    output.color = float4(1, 0, 0, 1);

    return output;
}