set(VkFiles
        VK/VkGraphicsContext.hpp VK/VkGraphicsContext.cpp)

set(CommonFiles
        Common/GraphicsContext.cpp Common/GraphicsContext.hpp
        Common/Window.cpp Common/Window.hpp)

if (NOT GraphicsApi STREQUAL "VK")
    set(VkFiles Common/GraphicsCommon.hpp VK/HelperFunctions.hpp)
endif()

add_library(KryneEngineGraphics STATIC
        ${VkFiles}
        ${CommonFiles})
target_link_libraries(KryneEngineGraphics EASTL glfw)
target_include_directories(KryneEngineGraphics PUBLIC ${RootDir})

if (GraphicsApi STREQUAL "VK")
    set(ENV{VULKAN_SDK} "C:\\VulkanSDK\\1.3.204.1")
    message(STATUS "Finding and linking Vulkan lib")
    find_package(Vulkan REQUIRED FATAL_ERROR)

    message(STATUS "Vulkan include dirs: " ${Vulkan_INCLUDE_DIRS})
    target_include_directories(KryneEngineGraphics PUBLIC ${Vulkan_INCLUDE_DIRS})

    message(STATUS "Vulkan lib: " ${Vulkan_LIBRARIES})
    target_link_libraries(KryneEngineGraphics ${Vulkan_LIBRARIES})
endif()