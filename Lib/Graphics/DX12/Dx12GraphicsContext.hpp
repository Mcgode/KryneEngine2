/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#pragma once

#include <Graphics/Common/GraphicsCommon.hpp>

namespace KryneEngine
{
    class Window;

    class Dx12GraphicsContext
    {
    public:
        explicit Dx12GraphicsContext(const GraphicsCommon::ApplicationInfo& _appInfo);

        [[nodiscard]] Window* GetWindow() const;
    };
} // KryneEngine