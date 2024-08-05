/**
 * @file
 * @author Max Godefroy
 * @date 04/08/2024.
 */

#pragma once

#include <Common/Types.hpp>
#include <Common/BitUtils.hpp>
#include <EASTL/fixed_vector.h>

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

    struct ColorAttachmentBlendDesc
    {
        enum class BlendFactor: u8
        {
            Zero,
            One,
            SrcColor,
            InvSrcColor,
            SrcAlpha,
            InvSrcAlpha,
            DstColor,
            InvDstColor,
            DstAlpha,
            InvDstAlpha,
            SrcAlphaSaturate,
            FactorColor,
            InvFactorColor,
            FactorAlpha,
            InvFactorAlpha,
            Src1Color,
            InvSrc1Color,
            Src1Alpha,
            InvSrc1Alpha,
        };

        enum class BlendOp: u8
        {
            Add,
            Subtract,
            ReverseSubtract,
            Min,
            Max,
        };

        enum class WriteMask: u8
        {
            Red     = 1 << 0,
            Green   = 1 << 1,
            Blue    = 1 << 2,
            Alpha   = 1 << 3,
            All     = Red | Green | Blue | Alpha,
        };

        bool m_blendEnable = false;
        BlendFactor m_srcColor = BlendFactor::One;
        BlendFactor m_dstColor = BlendFactor::Zero;
        BlendOp m_colorOp = BlendOp::Add;
        BlendFactor m_srcAlpha = BlendFactor::One;
        BlendFactor m_dstAlpha = BlendFactor::Zero;
        BlendOp m_alphaOp = BlendOp::Add;
        WriteMask m_writeMak = WriteMask::All;
    };
    KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(ColorAttachmentBlendDesc::WriteMask)

    struct ColorBlendingDesc
    {
        enum class LogicOp
        {
            None = 0,
            Clear,
            Set,
            Copy,
            CopyInverted,
            NoOp,
            Invert,
            And,
            NAnd,
            Or,
            NOr,
            XOr,
            Equiv,
            AndReverse,
            AndInverted,
            OrReverse,
            OrInverted,
        };

        eastl::fixed_vector<ColorAttachmentBlendDesc, 8, false> m_attachments {};
        float4 m_blendFactor = float4(0);
        LogicOp m_logicOp = LogicOp::None;
    };

    struct GraphicsPipelineDesc
    {
        InputAssemblyDesc m_inputAssembly;
        ColorBlendingDesc m_colorBlending;
    };
}
