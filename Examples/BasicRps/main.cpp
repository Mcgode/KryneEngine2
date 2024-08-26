/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include <Graphics/Common/GraphicsContext.hpp>
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
            const u64 frameIndex = graphicsContext->GetFrameId();

            const RpsRenderGraphUpdateInfo updateInfo {
                .frameIndex = graphicsContext->GetFrameId(),
                .gpuCompletedFrameIndex = (frameIndex > graphicsContext->GetFrameContextCount())
                    ? frameIndex - graphicsContext->GetFrameContextCount()
                    : RPS_GPU_COMPLETED_FRAME_INDEX_NONE,
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
