/**
 * @file
 * @author Max Godefroy
 * @date 02/08/2024.
 */

#pragma once

#include "KryneEngine/Core/Common/BitUtils.hpp"
#include "KryneEngine/Core/Graphics/Common/Enums.hpp"
#include "KryneEngine/Core/Graphics/Common/Handles.hpp"
#include "KryneEngine/Core/Memory/GenerationalPool.hpp"

namespace KryneEngine
{
    enum class BarrierSyncStageFlags : u32
    {
        None                        = 1 << 0,
        All                         = 1 << 1,
        ExecuteIndirect             = 1 << 2,
        IndexInputAssembly          = 1 << 3,
        VertexInputAssembly         = 1 << 4,
        VertexShading               = 1 << 5,
        FragmentShading             = 1 << 6,
        ColorBlending               = 1 << 7,
        DepthStencilTesting         = 1 << 8,
        Transfer                    = 1 << 9,
        MultiSampleResolve          = 1 << 10,
        ComputeShading              = 1 << 11,
        AllShading                  = 1 << 12,
        Raytracing                  = 1 << 13,
        AccelerationStructureBuild  = 1 << 14,
        AccelerationStructureCopy   = 1 << 15,
    };
    KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(BarrierSyncStageFlags)

    enum class BarrierAccessFlags: u32
    {
        VertexBuffer                = 1 << 0,
        IndexBuffer                 = 1 << 1,
        ConstantBuffer              = 1 << 2,
        IndirectBuffer              = 1 << 3,
        ColorAttachment             = 1 << 4,
        DepthStencilRead            = 1 << 5,
        DepthStencilWrite           = 1 << 5,
        ShaderResource              = 1 << 6,
        UnorderedAccess             = 1 << 7,
        ResolveSrc                  = 1 << 8,
        ResolveDst                  = 1 << 9,
        TransferSrc                 = 1 << 10,
        TransferDst                 = 1 << 11,
        AccelerationStructureRead   = 1 << 12,
        AccelerationStructureWrite  = 1 << 13,
        ShadingRate                 = 1 << 14,
        AllRead                     = 1 << 15,
        AllWrite                    = 1 << 16,
        All                         = AllRead | AllWrite,
        None                        = 1 << 17,
    };
    KE_ENUM_IMPLEMENT_BITWISE_OPERATORS(BarrierAccessFlags)

    struct GlobalMemoryBarrier
    {
        BarrierSyncStageFlags m_stagesSrc;
        BarrierSyncStageFlags m_stagesDst;
        BarrierAccessFlags m_accessSrc;
        BarrierAccessFlags m_accessDst;
    };

    struct BufferMemoryBarrier
    {
        BarrierSyncStageFlags m_stagesSrc;
        BarrierSyncStageFlags m_stagesDst;
        BarrierAccessFlags m_accessSrc;
        BarrierAccessFlags m_accessDst;

        u64 m_offset = 0;
        u64 m_size = eastl::numeric_limits<u64>::max();
        BufferHandle m_buffer;
    };

    struct TextureMemoryBarrier
    {
        BarrierSyncStageFlags m_stagesSrc;
        BarrierSyncStageFlags m_stagesDst;
        BarrierAccessFlags m_accessSrc;
        BarrierAccessFlags m_accessDst;

        TextureHandle m_texture;
        u16 m_arrayStart = 0;
        u16 m_arrayCount = 1;
        TextureLayout m_layoutSrc;
        TextureLayout m_layoutDst;
        u8 m_mipStart = 0;
        u8 m_mipCount = 1;

        TexturePlane m_planes = TexturePlane::Color;
    };
}
