set(VkFiles
        VK/VkGraphicsContext.hpp VK/VkGraphicsContext.cpp)

set(CommonFiles
        Common/GraphicsContext.cpp Common/GraphicsContext.hpp
        Common/Window.cpp Common/Window.hpp)

if (NOT GraphicsApi STREQUAL "VK")
    set(VkFiles)
endif()

add_library(KryneEngineGraphics STATIC
        ${VkFiles}
        ${CommonFiles})
target_link_libraries(KryneEngineGraphics EASTL glfw)
target_include_directories(KryneEngineGraphics PUBLIC ${RootDir})