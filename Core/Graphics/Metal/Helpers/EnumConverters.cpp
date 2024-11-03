/**
 * @file
 * @author Max Godefroy
 * @date 02/11/2024.
 */

#include "EnumConverters.hpp"

namespace KryneEngine::MetalConverters
{
    size_t GetPixelByteSize(TextureFormat _format)
    {
        static_assert(static_cast<u32>(TextureFormat::D32FS8) == 21, "Enum values changed, please update");

        switch (_format)
        {
        case TextureFormat::NoFormat:
        case TextureFormat::D24:
        case TextureFormat::RGB32_Float:
            return 0;
        case TextureFormat::R8_UNorm:
        case TextureFormat::R8_SNorm:
            return 1;
        case TextureFormat::RG8_UNorm:
        case TextureFormat::RG8_SNorm:
        case TextureFormat::D16:
            return 2;
        case TextureFormat::RGB8_UNorm:
        case TextureFormat::RGB8_sRGB:
        case TextureFormat::RGB8_SNorm:
            return 3;
        case TextureFormat::RGBA8_UNorm:
        case TextureFormat::RGBA8_sRGB:
        case TextureFormat::BGRA8_UNorm:
        case TextureFormat::BGRA8_sRGB:
        case TextureFormat::RGBA8_SNorm:
        case TextureFormat::R32_Float:
        case TextureFormat::D24S8:
        case TextureFormat::D32F:
            return 4;
        case TextureFormat::RG32_Float:
        case TextureFormat::D32FS8:
            return 8;
        case TextureFormat::RGBA32_Float:
            return 16;
        }
    }

    MTL::LoadAction GetMetalLoadOperation(RenderPassDesc::Attachment::LoadOperation _op)
    {
        switch (_op)
        {
        case RenderPassDesc::Attachment::LoadOperation::Load:
            return MTL::LoadActionLoad;
        case RenderPassDesc::Attachment::LoadOperation::Clear:
            return MTL::LoadActionClear;
        case RenderPassDesc::Attachment::LoadOperation::DontCare:
            return MTL::LoadActionDontCare;
        }
    }

    MTL::StoreAction GetMetalStoreOperation(RenderPassDesc::Attachment::StoreOperation _op)
    {
        switch (_op)
        {
        case RenderPassDesc::Attachment::StoreOperation::Store:
            return MTL::StoreActionStore;
        case RenderPassDesc::Attachment::StoreOperation::Resolve:
            return MTL::StoreActionStoreAndMultisampleResolve;
        case RenderPassDesc::Attachment::StoreOperation::DontCare:
            return MTL::StoreActionDontCare;
        }
    }
}