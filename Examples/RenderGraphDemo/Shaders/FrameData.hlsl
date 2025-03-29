/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#pragma once

struct FrameData {
    float4x4 m_torusWorldMatrix;

    float4x4 m_viewProjectionMatrix;

    float3 m_torusAlbedo;
    uint m_padding0;

    float3 m_sunLightDirection;
    uint m_padding1;

    float3 m_sunDiffuse;
    uint m_padding2;
};