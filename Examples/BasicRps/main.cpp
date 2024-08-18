/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include <RpsRuntime/Public/RpsRuntime.hpp>
#include <Window/Window.hpp>

using namespace KryneEngine;
namespace KEModules = KryneEngine::Modules;

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

    RpsDevice device;
    {
        KE_ZoneScoped("RPS device init");

        const KEModules::RpsRuntime::RuntimeDeviceCreateInfo createInfo{
            .m_graphicsContext = window.GetGraphicsContext(),
        };

        KEModules::RpsRuntime::rpsRuntimeDeviceCreate(&createInfo, &device);
    }
}
