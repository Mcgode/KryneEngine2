/**
 * @file
 * @author Max Godefroy
 * @date 08/01/2026.
 */

#pragma once

#include <EASTL/string_view.h>
#include <KryneEngine/Core/Math/Color.hpp>
#include <KryneEngine/Core/Math/Vector.hpp>
#include <fstream>

namespace KryneEngine::Tests
{
    class SvgDump
    {
    public:
        SvgDump(eastl::string_view _path, eastl::string_view _name, uint2 _size);

        void AddRect(
            double _x,
            double _y,
            double _width,
            double _height,
            double _strokeWidth,
            const Color& _color,
            const Color& _strokeColor);

        ~SvgDump();

    private:
        std::ofstream m_file;
    };
}
