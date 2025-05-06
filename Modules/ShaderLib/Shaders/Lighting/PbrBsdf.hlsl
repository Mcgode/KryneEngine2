/**
 * @file
 * @author Max Godefroy
 * @date 06/05/2025.
 */

#pragma once

#include "Math/Constants.hlsl"
 
float3 fresnelSchlick(const in float _nDotV, const in float3 _specularColor)
{
    // Original approximation by Christophe Schlick '94
    // float fresnel = pow( 1.0 - cosTheta, 5.0 );

    // Optimized variant (presented by Epic at SIGGRAPH '13)
    // https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
    const float fresnel = exp2( ( -5.55473 * _nDotV - 6.98316 ) * _nDotV );

    return _specularColor + (1.0 - _specularColor) * fresnel;
}


float3 RoughFresnelSchlick(const in float _nDotV, const in float3 _specularColor, const float _roughness)
{
    // See fresnelSchlick()
    float fresnel = exp2( ( -5.55473 * _nDotV - 6.98316 ) * _nDotV );
    float3 Fr = max((1 - _roughness).xxx, _specularColor) - _specularColor;

    return _specularColor + Fr * fresnel;
}


// Microfacet Models for Refraction through Rough Surfaces - equation (33)
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// alpha is "roughness squared" in Disney’s reparameterization
float DistributionGGX(const in float _nDotH, const in float _alpha)
{
    const float alphaSq = _alpha * _alpha;
    const float nDotHSq = _nDotH * _nDotH;

    const float denom = nDotHSq * (alphaSq - 1.0) + 1.0;

    return alphaSq / (kPi * denom * denom);
}


// Microfacet Models for Refraction through Rough Surfaces - equation (34)
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// alpha is "roughness squared" in Disney’s reparameterization
float GeometrySmithGGX(const in float nDotV, const in float nDotL, const in float alpha)
{
    const float a2 = alpha * alpha;

    const float gl = nDotL + sqrt( a2 + ( 1.0 - a2 ) * nDotL * nDotL );
    const float gv = nDotV + sqrt( a2 + ( 1.0 - a2 ) * nDotV * nDotV );

    return 1.0 / (gl * gv);
}


// Moving Frostbite to Physically Based Rendering 3.0 - page 12, listing 2
// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
float GeometrySmithGGXCorrelated(const in float nDotV, const in float nDotL, const in float alpha)
{
    const float a2 = alpha * alpha;

    // nDotL and nDotV are explicitly swapped. This is not a mistake.
    const float gl = nDotV + sqrt( a2 + ( 1.0 - a2 ) * nDotL * nDotL );
    const float gv = nDotL + sqrt( a2 + ( 1.0 - a2 ) * nDotV * nDotV );

    return 0.5 / max( gv + gl, 0.001 );
}
 
float3 BRDFSpecularGGX(
    const in float3 lightDirection,
    const in float3 viewDir,
    const in float3 normal,
    const in float3 specularColor,
    const in float roughness)
{
    const float3 halfDir = normalize(lightDirection + viewDir);

    const float alpha    = roughness * roughness;
    const float cosTheta = max(0, dot(normal, lightDirection));

    const float nDotV = max(0, dot(normal, viewDir));
    const float nDotL = max(0, dot(normal, lightDirection));
    const float nDotH = max(0, dot(normal, halfDir));
    const float lDotH = max(0, dot(halfDir, viewDir));

    const float3 F = fresnelSchlick(lDotH, specularColor);

    const float NDF = DistributionGGX(nDotH, alpha);
    const float G   = GeometrySmithGGXCorrelated(nDotV, nDotL, alpha);

    return NDF * G * F;
}