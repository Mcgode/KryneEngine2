/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#pragma once

#include <EASTL/span.h>
#include <Graphics/Common/Enums.hpp>
#include <Graphics/Common/GraphicsCommon.hpp>
#include <Common/KETypes.hpp>

namespace KryneEngine
{
    class TextureMemory;

    class Texture
    {
    public:
        struct Options
        {
            TextureFormat m_format = TextureFormat::RGBA8_sRGB;
            TextureTypes m_type = TextureTypes::Single2D;

            static constexpr eastl::array<TextureAspectType, 1> kDefaultAspectType = { TextureAspectType::Color };
            eastl::span<const TextureAspectType> m_textureAspect { kDefaultAspectType };

            u32 m_baseMipLevel = 0;
            u32 m_mipLevelCount = 1;

            u32 m_baseArrayLayer = 0;
            u32 m_arrayLayerCount = 1;
        };

        virtual ~Texture() = default;

        [[nodiscard]] TextureFormat GetFormat() const { return m_format; }

        /// @brief Returns `false` if the class owns memory
        [[nodiscard]] virtual bool IsRef() const = 0;

    protected:
        TextureFormat m_format = TextureFormat::NoFormat;

        glm::uvec2 m_size;
    };
}
