/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2025.
 */

namespace Quaternion
{
    float3 Apply(const in float4 _quaternion, const in float3 _vector)
    {
        return 2.f * _quaternion.xyz * dot(_vector, _quaternion.xyz)
            + _vector * (_quaternion.w * _quaternion.w - dot(_quaternion.xyz, _quaternion.xyz))
            + cross(_quaternion.xyz, _vector) * 2.f * _quaternion.w;
    }

    float4 Conjugate(const in float4 _quaternion)
    {
        return float4(-_quaternion.xyz, _quaternion.w);
    }
}