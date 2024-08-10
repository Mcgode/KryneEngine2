/**
 * @file
 * @author Max Godefroy
 * @date 04/08/2024.
 */

#pragma once

#include "Enums.hpp"
#include "Handles.hpp"
#include <Common/BitUtils.hpp>
#include <Common/Types.hpp>
#include <EASTL/fixed_vector.h>

namespace KryneEngine
{
    struct GraphicsShaderStage
    {
        enum class Stage : u8
        {
            Vertex,
            TesselationControl,
            TesselationEvaluation,
            Geometry,
            Fragment,
        };

        ShaderModuleHandle m_shaderModule { GenPool::kInvalidHandle };
        Stage m_stage = Stage::Vertex;
    };

    struct VertexLayoutElement
    {
        enum class SemanticName : u8
        {
            Position,
            Normal,
            Uv,
            Color,
            Tangent,
            BiTangent,
            BoneIndices,
            BoneWeights,
        };

        SemanticName m_semanticName = SemanticName::Position;
        u8 m_semanticIndex : 4 = 0;
        u8 m_bindingIndex : 4 = 0;
        TextureFormat m_format = TextureFormat::RGBA8_UNorm;
        u16 m_offset = 0;
    };

    struct InputAssemblyDesc
    {
        enum class PrimitiveTopology: u8
        {
            PointList,
            LineList,
            LineStrip,
            TriangleList,
            TriangleStrip,
        };

        enum class IndexIntSize: u8
        {
            U16,
            U32,
        };

        PrimitiveTopology m_topology = PrimitiveTopology::TriangleList;
        IndexIntSize m_indexSize = IndexIntSize::U32;
        bool m_cutStripAtSpecialIndex = false;
    };

    struct RasterStateDesc
    {
        enum class FillMode: u8
        {
            Wireframe,
            Solid
        };

        enum class CullMode: u8
        {
            None,
            Front,
            Back,
        };

        enum class Front
        {
            Clockwise,
            CounterClockwise,
        };

        FillMode m_fillMode = FillMode::Solid;
        CullMode m_cullMode = CullMode::Back;
        Front m_front = Front::CounterClockwise;
        bool m_depthClip = true;
        bool m_depthBias = false;
        float m_depthBiasConstantFactor = 0;
        float m_depthBiasSlopFactor = 0;
        float m_depthBiasClampValue = 0;
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

    static constexpr ColorAttachmentBlendDesc kDefaultColorAttachmentOpaqueBlendDesc {};
    static constexpr ColorAttachmentBlendDesc kDefaultColorAttachmentAlphaBlendDesc {
        .m_blendEnable = true,
        .m_srcColor = ColorAttachmentBlendDesc::BlendFactor::SrcAlpha,
        .m_dstColor = ColorAttachmentBlendDesc::BlendFactor::InvSrcAlpha,
        .m_colorOp = ColorAttachmentBlendDesc::BlendOp::Add,
        .m_srcAlpha = ColorAttachmentBlendDesc::BlendFactor::One,
        .m_dstAlpha = ColorAttachmentBlendDesc::BlendFactor::InvSrcAlpha,
        .m_alphaOp = ColorAttachmentBlendDesc::BlendOp::Add,
    };

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
        bool m_dynamicBlendFactor = false;
    };

    struct DepthStencilStateDesc
    {
        enum class CompareOp : u8
        {
            Never,
            Less,
            Equal,
            LessEqual,
            Greater,
            NotEqual,
            GreaterEqual,
            Always,
        };

        enum class StencilOp : u8
        {
            Keep,
            Zero,
            Replace,
            IncrementAndClamp,
            DecrementAndClamp,
            Invert,
            IncrementAndWrap,
            DecrementAndWrap,
        };

        struct StencilOpState
        {
            StencilOp m_passOp = StencilOp::Keep;
            StencilOp m_failOp = StencilOp::Keep;
            StencilOp m_depthFailOp = StencilOp::Keep;
            CompareOp m_compareOp = CompareOp::Never;
        };

        bool m_depthTest = true;
        bool m_depthWrite = true;
        CompareOp m_depthCompare = CompareOp::Less;
        bool m_stencilTest = false;

        u8 m_stencilReadMask = 0xFF;
        u8 m_stencilWriteMask = 0xFF;
        u8 m_stencilRef = 0xFF;
        bool m_dynamicStencilRef = false;

        StencilOpState m_front {};
        StencilOpState m_back {};
    };

    enum class ShaderVisibility: u8
    {
        Vertex = 1 << 0,
        TesselationControl = 1 << 1,
        TesselationEvaluation = 1 << 2,
        Geometry = 1 << 3,
        Fragment = 1 << 4,
        Compute = 1 << 5,
        Task = 1 << 6,
        Mesh = 1 << 7,

        All = 0xFF,
    };
    KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(ShaderVisibility)

    struct PushConstantDesc
    {
        u8 m_sizeInBytes = 0;
        u8 m_offset = 0;
        u8 m_index = 0;
        ShaderVisibility m_visibility = ShaderVisibility::All;
    };

    struct PipelineLayoutDesc
    {
        eastl::vector<DescriptorSetHandle> m_descriptorSets {};
        eastl::vector<PushConstantDesc> m_pushConstants {};
        bool m_useVertexLayout = true;
    };

    struct GraphicsPipelineDesc
    {
        eastl::vector<GraphicsShaderStage> m_stages {};
        eastl::vector<VertexLayoutElement> m_vertexLayout {};
        InputAssemblyDesc m_inputAssembly {};
        RasterStateDesc m_rasterState {};
        ColorBlendingDesc m_colorBlending {};
        DepthStencilStateDesc m_depthStencil {};
        RenderPassHandle m_renderPass { GenPool::kInvalidHandle };
        PipelineLayoutHandle m_pipelineLayout { GenPool::kInvalidHandle };

#if !defined(KE_FINAL)
        eastl::string m_debugName = "";
#endif
    };
}
