/**
 * @file
 * @author Max Godefroy
 * @date 02/11/2024.
 */

#pragma once

#include <Graphics/Common/EnumHelpers.hpp>
#include <Graphics/Common/RenderPass.hpp>
#include <Graphics/Metal/MetalHeaders.hpp>

namespace KryneEngine::MetalConverters
{
    [[nodiscard]] size_t GetPixelByteSize(TextureFormat _format);

    [[nodiscard]] MTL::ResourceOptions GetResourceStorage(MemoryUsage _memoryUsage);

    [[nodiscard]] MTL::LoadAction GetMetalLoadOperation(RenderPassDesc::Attachment::LoadOperation _op);
    [[nodiscard]] MTL::StoreAction GetMetalStoreOperation(RenderPassDesc::Attachment::StoreOperation _op);
}