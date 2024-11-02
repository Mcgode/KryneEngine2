/**
 * @file
 * @author Max Godefroy
 * @date 02/11/2024.
 */

#pragma once

#include <Graphics/Common/RenderPass.hpp>
#include <Graphics/Metal/MetalHeaders.hpp>

namespace KryneEngine::MetalConverters
{
    [[nodiscard]] MTL::LoadAction GetMetalLoadOperation(RenderPassDesc::Attachment::LoadOperation _op);
    [[nodiscard]] MTL::StoreAction GetMetalStoreOperation(RenderPassDesc::Attachment::StoreOperation _op);
}