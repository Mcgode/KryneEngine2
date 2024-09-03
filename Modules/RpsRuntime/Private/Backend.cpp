/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include "Backend.hpp"

#include "Device.hpp"
#include "Helpers.hpp"

#include <Graphics/Common/Buffer.hpp>
#include <Graphics/Common/GraphicsContext.hpp>

namespace KryneEngine::Modules::RpsRuntime
{
    Backend::Backend(Device& _device, rps::RenderGraph& _renderGraph)
        : rps::RuntimeBackend(_renderGraph)
        , m_device(_device)
    {}

    RpsResult Backend::RecordCommands(
        const rps::RenderGraph& renderGraph, const RpsRenderGraphRecordCommandInfo& recordInfo) const
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Backend::RecordCmdRenderPassBegin(const rps::RuntimeCmdCallbackContext& context) const
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Backend::RecordCmdRenderPassEnd(const rps::RuntimeCmdCallbackContext& context) const
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    RpsResult Backend::RecordCmdFixedFunctionBindingsAndDynamicStates(const rps::RuntimeCmdCallbackContext& context) const
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    void Backend::DestroyRuntimeResourceDeferred(rps::ResourceInstance& resource)
    {
        KE_ERROR("Not implemented");
    }

    RpsResult Backend::UpdateFrame(const rps::RenderGraphUpdateContext& context)
    {
        // Handle frame data here
        // There are none so far

        return RPS_OK;
    }

    RpsResult Backend::CreateHeaps(const rps::RenderGraphUpdateContext& context, rps::ArrayRef<rps::HeapInfo> heaps)
    {
        // Heaps not handled for now
        return RPS_OK;
    }

    void Backend::DestroyHeaps(rps::ArrayRef<rps::HeapInfo> heaps)
    {
        // Heaps not implemented for now
    }

    RpsResult Backend::CreateResources(const rps::RenderGraphUpdateContext& context, rps::ArrayRef<rps::ResourceInstance> resources)
    {
        rps::ConstArrayRef<rps::ResourceDecl, u32> resourceDeclarations = GetRenderGraph().GetBuilder().GetResourceDecls();

        const bool enableDebugNames = BitUtils::EnumHasAny(
            static_cast<RpsDiagnosticFlagBits>(context.pUpdateInfo->diagnosticFlags),
            RPS_DIAGNOSTIC_ENABLE_RUNTIME_DEBUG_NAMES);

        for (auto& resourceInstance: resources)
        {
            if (resourceInstance.isPendingCreate)
            {
                const char* name = enableDebugNames
                    ? resourceDeclarations[resourceInstance.resourceDeclId].name.str
                    : "";

                if (resourceInstance.desc.IsBuffer())
                {
                    BufferCreateDesc createDesc {
                        .m_desc = {
                            .m_size = resourceInstance.desc.GetBufferSize(),
#if !defined(KE_FINAL)
                            .m_debugName = name,
#endif
                        },
                        .m_usage = ToKeBufferMemoryUsage(resourceInstance.allAccesses.accessFlags)
                                   & ToKeHeapMemoryType(resourceInstance),
                    };
                    const BufferHandle handle = m_device.GetGraphicsContext()->CreateBuffer(createDesc);
                    resourceInstance.hRuntimeResource = ToRpsHandle<BufferHandle, RpsRuntimeResource>(handle);
                }
                else
                {
                    const u32 depth = resourceInstance.desc.type == RPS_RESOURCE_TYPE_IMAGE_3D
                                          ? resourceInstance.desc.image.depth : 1;

                    TextureDesc textureDesc {
                        .m_dimensions = {
                            resourceInstance.desc.image.width,
                            resourceInstance.desc.image.height,
                            depth
                        },
                        .m_format = ToKeTextureFormat(resourceInstance.desc.GetFormat()),
                        .m_arraySize = static_cast<u16>(resourceInstance.desc.type != RPS_RESOURCE_TYPE_IMAGE_3D
                            ? resourceInstance.desc.image.arrayLayers : 1u),
                        .m_type = TextureTypes::Single2D,
                        .m_mipCount = static_cast<u8>(resourceInstance.desc.image.mipLevels),
                        .m_planes = GetAspectMaskFromFormat(resourceInstance.desc.GetFormat()),
#if !defined(KE_FINAL)
                        .m_debugName = name,
#endif
                    };

                    const TextureCreateDesc createDesc {
                        .m_desc = textureDesc,
                        .m_footprintPerSubResource =
                            m_device.GetGraphicsContext()->FetchTextureSubResourcesMemoryFootprints(textureDesc),
                        .m_memoryUsage = ToKeTextureMemoryUsage(resourceInstance.allAccesses.accessFlags)
                                         & ToKeHeapMemoryType(resourceInstance),
                    };

                    const TextureHandle handle = m_device.GetGraphicsContext()->CreateTexture(createDesc);
                    resourceInstance.hRuntimeResource = ToRpsHandle<TextureHandle, RpsRuntimeResource>(handle);
                }
            }
            else if (!resourceInstance.isExternal)
            {
                resourceInstance.isPendingInit = resourceInstance.isAliased;
            }
        }
        return RPS_OK;
    }

    void Backend::DestroyResources(rps::ArrayRef<rps::ResourceInstance> resources)
    {
        for (auto& resInfo: resources)
        {
            if (resInfo.hRuntimeResource.ptr != nullptr && !resInfo.isExternal)
            {
                GraphicsContext* graphicsContext = m_device.GetGraphicsContext();

                if (resInfo.desc.IsImage())
                {
                    graphicsContext->DestroyTexture(ToKeHandle<TextureHandle>(resInfo.hRuntimeResource));
                }
                else if (resInfo.desc.IsBuffer())
                {
                    graphicsContext->DestroyBuffer(ToKeHandle<BufferHandle>(resInfo.hRuntimeResource));
                }
            }
        }
    }

    RpsResult Backend::CreateCommandResources(const rps::RenderGraphUpdateContext& context)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }

    void Backend::DestroyCommandResources()
    {
        KE_ERROR("Not implemented");
        RuntimeBackend::DestroyCommandResources();
    }

    void Backend::RecordDebugMarker(
        const rps::RuntimeCmdCallbackContext& context,
        RpsRuntimeDebugMarkerMode mode, rps::StrRef name) const
    {
        KE_ERROR("Not implemented");
        RuntimeBackend::RecordDebugMarker(context, mode, name);
    }

    bool Backend::ShouldResetAliasedResourcesPrevFinalAccess() const
    {
        KE_ERROR("Not implemented");
        return RuntimeBackend::ShouldResetAliasedResourcesPrevFinalAccess();
    }
} // namespace KryneEngine::Modules::RpsRuntime