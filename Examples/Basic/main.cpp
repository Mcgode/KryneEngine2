#include <iostream>
#include <Graphics/Common/GraphicsContext.hpp>
#include <Threads/FibersManager.hpp>

int main() {
    std::cout << "Hello, World!" << std::endl;

    auto appInfo = KryneEngine::GraphicsCommon::ApplicationInfo();
    appInfo.m_api = KryneEngine::GraphicsCommon::Api::Vulkan_1_2;
    KryneEngine::GraphicsContext graphicsContext(appInfo);

    {
        auto fibersManager = KryneEngine::FibersManager(6);
    }

    while (graphicsContext.EndFrame());

    return 0;
}
