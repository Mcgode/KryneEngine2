/**
 * @file
 * @author Max Godefroy
 * @date 19/06/2025.
 */

#include "KryneEngine/Modules/SdfTexture/Generator.hpp"

#include <cmath>

namespace KryneEngine::Modules::SdfTexture
{
    Generator::Generator(AllocatorInstance _allocator)
        : m_allocator(_allocator)
    {}

    void Generator::ForceDimensions(const uint3& _dimensions)
    {
        KE_ASSERT(_dimensions.x > kMinDimension && _dimensions.y > kMinDimension && _dimensions.z > kMinDimension);

        const float3 texelSizes = m_boundingBox.GetSize() / float3(_dimensions - 1);
        m_texelSize = std::fmax(texelSizes.x, std::fmax(texelSizes.y, texelSizes.z));
        m_dimensions = _dimensions;
    }

    void Generator::ComputeDimensionsFromBudget(u32 _texelBudget)
    {
        KE_ASSERT(m_boundingBox.IsValid());
        KE_ASSERT(_texelBudget > kMinDimension * kMinDimension * kMinDimension);

        const float3 size = m_boundingBox.GetSize();
        KE_ASSERT(size.x > 0 && size.y > 0 && size.z > 0);

        u32 biggestComponent;
        if (size.x >= size.y)
        {
            biggestComponent = size.x >= size.z ? 0 : 2;
        }
        else
        {
            biggestComponent = size.y >= size.z ? 1 : 2;
        }
        const float3 relativeSize = size / size[biggestComponent];

        const float k = std::pow(float(_texelBudget) / (relativeSize.x * relativeSize.y * relativeSize.z), 1.f / 3.f);
        float referential = std::round(std::fmin(k, float(_texelBudget) / (kMinDimension * kMinDimension)));

        u32 totalUsedTexels = 0;

        while (referential > kMinDimension)
        {
            // The -1 and +1 are for setting up a half texel border to the texture.

            const float texelSize = size[biggestComponent] / (referential - 1);
            float3 sdfSize {};
            totalUsedTexels = static_cast<u32>(referential);
            m_dimensions[biggestComponent] = static_cast<u32>(referential);

            {
                const u32 index = (biggestComponent + 1) % 3;
                const float requiredTexelCount = std::fmax(std::ceil(size[index] / texelSize) + 1.f, float(kMinDimension));
                totalUsedTexels *= static_cast<u32>(requiredTexelCount);
                m_dimensions[index] = static_cast<u32>(requiredTexelCount);
            }

            {
                const u32 index = (biggestComponent + 2) % 3;
                const float requiredTexelCount = std::fmax(std::ceil(size[index] / texelSize) + 1.f, float(kMinDimension));
                totalUsedTexels *= static_cast<u32>(requiredTexelCount);
                m_dimensions[index] = static_cast<u32>(requiredTexelCount);
            }

            if (totalUsedTexels <= _texelBudget)
            {
                m_texelSize = texelSize;
                break;
            }

            referential -= 1.f;
        }
        KE_ASSERT(totalUsedTexels <= _texelBudget && referential > kMinDimension);
    }
}