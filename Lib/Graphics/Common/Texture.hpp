/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#pragma once

#include <Graphics/Common/Enums.hpp>
#include <Common/KETypes.hpp>

namespace KryneEngine
{
    class TextureMemory;

    class Texture
    {
    public:
        virtual ~Texture() = default;

        [[nodiscard]] TextureFormat GetFormat() const { return m_format; }

        /// @brief Returns `false` if the class owns memory
        [[nodiscard]] virtual bool IsRef() const = 0;

    protected:
        TextureFormat m_format = TextureFormat::NoFormat;

        glm::vec2 m_size;
    };
}
