#include <iostream>
#include <Graphics/Common/GraphicsContext.hpp>

int main() {
    std::cout << "Hello, World!" << std::endl;

    auto appInfo = KryneEngine::GraphicsCommon::ApplicationInfo();
    appInfo.m_api = KryneEngine::GraphicsCommon::Api::Vulkan_1_2;
    KryneEngine::GraphicsContext graphicsContext(appInfo);

    while (graphicsContext.EndFrame());

    return 0;
}
