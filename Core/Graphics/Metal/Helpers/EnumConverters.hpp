/**
 * @file
 * @author Max Godefroy
 * @date 02/11/2024.
 */

#pragma once

#include <Graphics/Common/EnumHelpers.hpp>
#include <Graphics/Common/RenderPass.hpp>
#include <Graphics/Common/ResourceViews/ShaderResourceView.hpp>
#include <Graphics/Common/ShaderPipeline.hpp>
#include <Graphics/Metal/MetalHeaders.hpp>

namespace KryneEngine::MetalConverters
{
    [[nodiscard]] size_t GetPixelByteSize(TextureFormat _format);
    [[nodiscard]] MTL::PixelFormat ToPixelFormat(TextureFormat _format);

    [[nodiscard]] MTL::ResourceOptions GetResourceStorage(MemoryUsage _memoryUsage);
    [[nodiscard]] MTL::StorageMode GetStorageMode(MemoryUsage _memoryUsage);
    [[nodiscard]] MTL::TextureUsage GetTextureUsage(MemoryUsage _usage);

    [[nodiscard]] MTL::TextureSwizzle GetSwizzle(TextureComponentMapping _mapping);

    [[nodiscard]] MTL::TextureType GetTextureType(TextureTypes _type);

    [[nodiscard]] MTL::DataType GetDataType(DescriptorBindingDesc::Type _type);
    [[nodiscard]] MTL::BindingAccess GetBindingAccess(DescriptorBindingDesc::Type _type);

    [[nodiscard]] MTL::LoadAction GetMetalLoadOperation(RenderPassDesc::Attachment::LoadOperation _op);
    [[nodiscard]] MTL::StoreAction GetMetalStoreOperation(RenderPassDesc::Attachment::StoreOperation _op);
}