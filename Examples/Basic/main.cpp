#include <iostream>
#include <Graphics/Common/GraphicsContext.hpp>

int main() {
    std::cout << "Hello, World!" << std::endl;

    KryneEngine::GraphicsContext graphicsContext;

    while (graphicsContext.EndFrame());

    return 0;
}
