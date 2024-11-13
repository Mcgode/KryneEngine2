/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include <Graphics/Common/GraphicsContext.hpp>
#include <Profiling/TracyHeader.hpp>
#include <RpsRuntime/Private/Helpers.hpp>
#include <RpsRuntime/Public/RpsRuntime.hpp>
#include <Window/Window.hpp>

using namespace KryneEngine;
namespace KEModules = KryneEngine::Modules;

RPS_DECLARE_RPSL_ENTRY(hello_triangle, main);

void DrawTriangleCallback(const RpsCmdCallbackContext* _context)
{
    KE_VERIFY(_context != nullptr);
}

int main()
{
    auto appInfo = GraphicsCommon::ApplicationInfo();
    appInfo.m_features.m_validationLayers = false;
    appInfo.m_applicationName = "Basic RPS - Kryne Engine 2";
#if defined(KE_GRAPHICS_API_VK)
    appInfo.m_api = GraphicsCommon::Api::Vulkan_1_3;
#elif defined(KE_GRAPHICS_API_DX12)
    appInfo.m_api = KryneEngine::GraphicsCommon::Api::DirectX12_1;
#endif

    Window window(appInfo);
    GraphicsContext* graphicsContext = window.GetGraphicsContext();

    RpsDevice device;
    {
        KE_ZoneScoped("RPS device init");

        const KEModules::RpsRuntime::RuntimeDeviceCreateInfo createInfo{
            .m_graphicsContext = window.GetGraphicsContext(),
        };

        KEModules::RpsRuntime::rpsRuntimeDeviceCreate(&createInfo, &device);
    }

    RpsRenderGraph renderGraph;
    {
        RpsQueueFlags queueFlags[] = { RPS_QUEUE_FLAG_GRAPHICS };
        const RpsRenderGraphCreateInfo createInfo {
            .scheduleInfo = {
                .numQueues = 1,
                .pQueueInfos = queueFlags,
            },
            .mainEntryCreateInfo = {
                .hRpslEntryPoint = RPS_ENTRY_REF(hello_triangle, main),
            },
        };

        KE_RPS_ASSERT(rpsRenderGraphCreate(device, &createInfo, &renderGraph));
    }

    KE_RPS_ASSERT(rpsProgramBindNode(
        rpsRenderGraphGetMainEntry(renderGraph),
        "Triangle",
        DrawTriangleCallback));

    do
    {
        {
            eastl::vector<RpsRuntimeResource> presentResources { graphicsContext->GetFrameContextCount() };
            for (auto i = 0u; i < graphicsContext->GetFrameContextCount(); i++)
            {
                presentResources.push_back(KEModules::RpsRuntime::ToRpsHandle<TextureHandle, RpsRuntimeResource>(
                    graphicsContext->GetPresentTexture(i)));
            }
            const RpsResourceDesc resourceDesc {
                .type = RPS_RESOURCE_TYPE_IMAGE_2D,
                .temporalLayers = static_cast<u32>(presentResources.size()),
                .image = {
                    .width = appInfo.m_displayOptions.m_width,
                    .height = appInfo.m_displayOptions.m_height,
                    .arrayLayers = 1,
                    .mipLevels = 1,
                    .format = RPS_FORMAT_R8G8B8A8_UNORM,
                    .sampleCount = 1,
                },
            };

            RpsConstant argData[] = { &resourceDesc };
            const RpsRuntimeResource* argResources[] = { presentResources.data() };

            const u64 frameIndex = graphicsContext->GetFrameId();

            const RpsRenderGraphUpdateInfo updateInfo {
                .frameIndex = graphicsContext->GetFrameId(),
                .gpuCompletedFrameIndex = (frameIndex > graphicsContext->GetFrameContextCount())
                    ? frameIndex - graphicsContext->GetFrameContextCount()
                    : RPS_GPU_COMPLETED_FRAME_INDEX_NONE,
                .numArgs = 1,
                .ppArgs = argData,
                .ppArgResources = argResources,
            };

            KE_RPS_ASSERT(rpsRenderGraphUpdate(renderGraph, &updateInfo));
        }

        // RPS render pass
        {
            RpsRenderGraphBatchLayout batchLayout{};
            KE_RPS_ASSERT(rpsRenderGraphGetBatchLayout(renderGraph, &batchLayout));

            KE_ASSERT_MSG(
                batchLayout.numCmdBatches == 1, "In a single-queue app, we expect there to be a single cmd batch.");
            KE_ASSERT_MSG(batchLayout.numFenceSignals == 0, "In a single queue app, we don't expect any fence signal");
        }
    }
    while (window.GetGraphicsContext()->EndFrame());

    return 0;
}
