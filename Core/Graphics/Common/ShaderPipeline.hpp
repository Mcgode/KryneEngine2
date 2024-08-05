/**
 * @file
 * @author Max Godefroy
 * @date 04/08/2024.
 */

#pragma once

#include <Common/Types.hpp>

namespace KryneEngine
{
    enum class PrimitiveTopology: u8
    {
        TriangleList,
        TriangleStrip,
    };

    struct InputAssemblyDesc
    {
        PrimitiveTopology m_topology = PrimitiveTopology::TriangleList;
    };

    struct GraphicsPipelineDesc
    {
        InputAssemblyDesc m_inputAssembly;
    };
}
