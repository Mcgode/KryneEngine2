/**
 * @file
 * @author Max Godefroy
 * @date 30/11/2025.
 */

#pragma once

#include "TextureRegion.hpp"

namespace KryneEngine::Modules::GuiLib
{
    class IGuiRenderer
    {
    public:
        virtual ~IGuiRenderer() = default;

        virtual void BeginFrame() = 0;

        virtual void RenderFrame() = 0;
    };
} // namespace KryneEngine
