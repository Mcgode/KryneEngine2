/**
 * @file
 * @author Max Godefroy
 * @date 19/05/2025.
 */

#pragma once

#include "KryneEngine/Core/Math/BoundingBox.hpp"
#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"

namespace KryneEngine::Modules::SdfTexture
{
    /**
     * @brief Generates 3D SDF textures for a provided mesh
     *
     * @details
     * The generated texture will have a uniform texel size in all axis directions, and will feature a half-texel border
     */
    class Generator
    {
    public:
        explicit Generator(AllocatorInstance _allocator);

        void SetMeshBoundingBox(const Math::BoundingBox& _boundingBox) { m_boundingBox = _boundingBox; }

        void ForceDimensions(const uint3& _dimensions);
        void ForceDimensions(u32 _dimension) { ForceDimensions(uint3(_dimension)); }
        void ComputeDimensionsFromBudget(u32 _texelBudget);

        static constexpr u32 kMinDimension = 8;

    private:
        AllocatorInstance m_allocator;
        uint3 m_dimensions {};
        float m_texelSize {};
        Math::BoundingBox m_boundingBox {};
    };
}