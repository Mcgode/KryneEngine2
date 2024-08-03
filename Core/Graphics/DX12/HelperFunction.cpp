/**
 * @file
 * @author Max Godefroy
 * @date 31/01/2024.
 */

#include "HelperFunctions.hpp"
#include <iostream>

namespace KryneEngine
{
    D3D12_BARRIER_SYNC Dx12Converters::ToDx12BarrierSync(BarrierSyncStageFlags _flags)
    {
        D3D12_BARRIER_SYNC flags = D3D12_BARRIER_SYNC_NONE;
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::None))
        {
            return D3D12_BARRIER_SYNC_NONE;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::All))
        {
            flags |= D3D12_BARRIER_SYNC_ALL;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::ExecuteIndirect))
        {
            flags |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::InputAssembly))
        {
            flags |= D3D12_BARRIER_SYNC_INDEX_INPUT;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::VertexShading))
        {
            flags |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::FragmentShading))
        {
            flags |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::ColorBlending))
        {
            flags |= D3D12_BARRIER_SYNC_RENDER_TARGET;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::DepthStencilTesting))
        {
            flags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::Transfer))
        {
            flags |= D3D12_BARRIER_SYNC_COPY;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::MultiSampleResolve))
        {
            flags |= D3D12_BARRIER_SYNC_RESOLVE;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::ComputeShading))
        {
            flags |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::AllShading))
        {
            flags |= D3D12_BARRIER_SYNC_ALL_SHADING;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::Raytracing))
        {
            flags |= D3D12_BARRIER_SYNC_RAYTRACING;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::AccelerationStructureBuild))
        {
            flags |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierSyncStageFlags::AccelerationStructureCopy))
        {
            flags |= D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        return flags;
    }

    D3D12_BARRIER_ACCESS Dx12Converters::ToDx12BarrierAccess(BarrierAccessFlags _flags)
    {
        D3D12_BARRIER_ACCESS flags = D3D12_BARRIER_ACCESS_NO_ACCESS;

        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::None))
        {
            return D3D12_BARRIER_ACCESS_NO_ACCESS;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::VertexBuffer))
        {
            flags |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::IndexBuffer))
        {
            flags |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::ConstantBuffer))
        {
            flags |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::IndirectBuffer))
        {
            flags |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::ColorAttachment))
        {
            flags |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::DepthStencilRead))
        {
            flags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::DepthStencilWrite))
        {
            flags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::ShaderResource))
        {
            flags |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::UnorderedAccess))
        {
            flags |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::ResolveSrc))
        {
            flags |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::ResolveDst))
        {
            flags |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::TransferSrc))
        {
            flags |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::TransferDst))
        {
            flags |= D3D12_BARRIER_ACCESS_COPY_DEST;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::AccelerationStructureRead))
        {
            flags |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::AccelerationStructureWrite))
        {
            flags |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::ShadingRate))
        {
            flags |= D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::AllRead))
        {
            return D3D12_BARRIER_ACCESS_COMMON;
        }
        if (BitUtils::EnumHasAny(_flags, BarrierAccessFlags::AllWrite))
        {
            return D3D12_BARRIER_ACCESS_COMMON;
        }

        return flags;
    }

    D3D12_BARRIER_LAYOUT Dx12Converters::ToDx12BarrierLayout(TextureLayout _layout)
    {
        switch (_layout)
        {
        case TextureLayout::Unknown:
            return D3D12_BARRIER_LAYOUT_UNDEFINED;
        case TextureLayout::Common:
            return D3D12_BARRIER_LAYOUT_COMMON;
        case TextureLayout::Present:
            return D3D12_BARRIER_LAYOUT_PRESENT;
        case TextureLayout::GenericRead:
            return D3D12_BARRIER_LAYOUT_GENERIC_READ;
        case TextureLayout::ColorAttachment:
            return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
        case TextureLayout::DepthStencilAttachment:
            return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
        case TextureLayout::DepthStencilReadOnly:
            return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
        case TextureLayout::UnorderedAccess:
            return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
        case TextureLayout::ShaderResource:
            return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
        case TextureLayout::TransferSrc:
            return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
        case TextureLayout::TransferDst:
            return D3D12_BARRIER_LAYOUT_COPY_DEST;
        case TextureLayout::ResolveSrc:
            return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
        case TextureLayout::ResolveDst:
            return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
        case TextureLayout::ShadingRate:
            return D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE;
        }
    }

    D3D12_RESOURCE_STATES Dx12Converters::RetrieveState(BarrierAccessFlags _access, TextureLayout _layout)
    {
        D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;
        using namespace BitUtils;

        D3D12_BARRIER_ACCESS access = Dx12Converters::ToDx12BarrierAccess(_access);

        if (access == D3D12_BARRIER_ACCESS_COMMON)
        {
            return D3D12_RESOURCE_STATE_COMMON;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_VERTEX_BUFFER | D3D12_BARRIER_ACCESS_CONSTANT_BUFFER))
        {
            states |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_INDEX_BUFFER))
        {
            states |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        }
        if (_layout == TextureLayout::ColorAttachment)
        {
            states |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_UNORDERED_ACCESS) || _layout == TextureLayout::UnorderedAccess)
        {
            states |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        }
        if (_layout == TextureLayout::DepthStencilAttachment)
        {
            states |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        }
        if (_layout == TextureLayout::DepthStencilReadOnly)
        {
            states |= D3D12_RESOURCE_STATE_DEPTH_READ;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_SHADER_RESOURCE) || _layout == TextureLayout::ShaderResource)
        {
            states |= D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT))
        {
            states |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_COPY_SOURCE) || _layout == TextureLayout::TransferSrc)
        {
            states |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_COPY_DEST) || _layout == TextureLayout::TransferDst)
        {
            states |= D3D12_RESOURCE_STATE_COPY_DEST;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_RESOLVE_SOURCE) || _layout == TextureLayout::ResolveSrc)
        {
            states |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_RESOLVE_DEST) || _layout == TextureLayout::ResolveDst)
        {
            states |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ | D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE))
        {
            states |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }
        if (EnumHasAny(access, D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE))
        {
            states |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
        }

        return states;
    }

    u8 GetTextureBytesPerPixel(DXGI_FORMAT _format)
    {
        switch(_format)
        {
        default:
        case DXGI_FORMAT_UNKNOWN:
        case DXGI_FORMAT_R1_UNORM:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_YUY2:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
        case DXGI_FORMAT_NV11:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8:
        case DXGI_FORMAT_A8P8:
        case DXGI_FORMAT_P208:
        case DXGI_FORMAT_V208:
        case DXGI_FORMAT_V408:
            KE_ERROR("Format not supported yet");
            return 0;
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 16;
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 12;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return 8;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return 4;
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return 2;
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
            return 1;
        }
    }

    void DebugLayerMessageCallback(
            D3D12_MESSAGE_CATEGORY _category,
            D3D12_MESSAGE_SEVERITY _severity,
            D3D12_MESSAGE_ID _id,
            LPCSTR _description,
            void* _context)
    {
        constexpr auto kMinimumSeverity = D3D12_MESSAGE_SEVERITY_WARNING;
        constexpr auto kMinimumAssertSeverity = D3D12_MESSAGE_SEVERITY_ERROR;

        if (_severity > kMinimumSeverity)
        {
            return;
        }

        eastl::string severityString;
        switch (_severity)
        {
            case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                severityString = "corruption";
                break;
            case D3D12_MESSAGE_SEVERITY_ERROR:
                severityString = "error";
                break;
            case D3D12_MESSAGE_SEVERITY_WARNING:
                severityString = "warning";
                break;
            case D3D12_MESSAGE_SEVERITY_INFO:
                severityString = "info";
                break;
            case D3D12_MESSAGE_SEVERITY_MESSAGE:
                severityString = "message";
                break;
        }

        std::cout << "Validation layer (" << severityString.c_str() << "): " << _description << std::endl;

        KE_ASSERT(_severity > kMinimumAssertSeverity);
    }
}
