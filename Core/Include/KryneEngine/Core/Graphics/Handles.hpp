/**
 * @file
 * @author Max Godefroy
 * @date 04/08/2024.
 */

#pragma once

#include "KryneEngine/Core/Memory/GenerationalPool.hpp"

namespace KryneEngine
{
    KE_GENPOOL_DECLARE_HANDLE(BufferHandle);
    KE_GENPOOL_DECLARE_HANDLE(TextureHandle);
    KE_GENPOOL_DECLARE_HANDLE(TextureSrvHandle);
    KE_GENPOOL_DECLARE_HANDLE(SamplerHandle);
    KE_GENPOOL_DECLARE_HANDLE(BufferCbvHandle);
    KE_GENPOOL_DECLARE_HANDLE(RenderTargetViewHandle);
    KE_GENPOOL_DECLARE_HANDLE(RenderPassHandle);
    KE_GENPOOL_DECLARE_HANDLE(ShaderModuleHandle);
    KE_GENPOOL_DECLARE_HANDLE(DescriptorSetLayoutHandle);
    KE_GENPOOL_DECLARE_HANDLE(DescriptorSetHandle);
    KE_GENPOOL_DECLARE_HANDLE(PipelineLayoutHandle);
    KE_GENPOOL_DECLARE_HANDLE(GraphicsPipelineHandle);
    KE_GENPOOL_DECLARE_HANDLE(ComputePipelineHandle);
}
