/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#pragma once

#include <Graphics/Common/Enums.hpp>

namespace KryneEngine
{
    class TextureMemory;

    class TextureView
    {
    public:
        [[nodiscard]] virtual TextureFormat GetFormat() const = 0;

    protected:
        void _SetMemory(TextureMemory* _memory);

    private:
        TextureMemory* m_memory = nullptr;
    };
}
