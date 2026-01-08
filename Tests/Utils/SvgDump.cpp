/**
 * @file
 * @author Max Godefroy
 * @date 08/01/2026.
 */

#include "SvgDump.hpp"

namespace KryneEngine::Tests
{
    SvgDump::SvgDump(eastl::string_view _path, eastl::string_view _name, uint2 _size)
    {
        m_file.open(_path.data(), std::ios::out);
        m_file << R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>)" << std::endl;
        m_file << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" width=")" << _size.x << R"(" height=")" << _size.y << R"(">)" << std::endl;

        if (!_name.empty())
        {
            m_file << R"(<title>)" << _name.data() << R"(</title>)" << std::endl;
        }
    }

    void SvgDump::AddRect(
        double _x,
        double _y,
        double _width,
        double _height,
        double _strokeWidth,
        const Color& _color,
        const Color& _strokeColor)
    {
        m_file
            << R"(<rect x=")" << _x
            << R"(" y=")" << _y
            << R"(" width=")" << _width
            << R"(" height=")" << _height
            << R"(" fill=")" << eastl::string().sprintf("#%08X", _color.ToRgba8(false)).c_str()
            << R"(" stroke=")" << eastl::string().sprintf("#%08X", _strokeColor.ToRgba8(false)).c_str()
            << R"(" stroke-width=")" << _strokeWidth << R"(" />)"
            << std::endl;
    }

    SvgDump::~SvgDump()
    {
        m_file << "</svg>";
        m_file.close();
    }
} // namespace KryneEngine::Tests