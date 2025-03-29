/**
 * @file
 * @author Max Godefroy
 * @date 29/03/2025.
 */

#include "SunLight.hpp"

namespace KryneEngine::Samples::RenderGraphDemo
{
    SunLight::SunLight()
    {
        m_direction = float3(0.2f, 0.1f, -1.0f).Normalized();
        m_color = float3(1.0f, 1.0f, 1.0f);
        m_intensity = 1.0f;
    }
}