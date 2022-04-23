set(VkFiles
        VK/VkGraphicsContext.hpp VK/VkGraphicsContext.cpp
        VK/HelperFunctions.hpp
        VK/VkSwapChain.cpp VK/VkSwapChain.hpp
        VK/VkSurface.cpp VK/VkSurface.hpp
        VK/CommonStructures.hpp)

set(CommonFiles
        Common/GraphicsContext.cpp Common/GraphicsContext.hpp
        Common/Window.cpp Common/Window.hpp
        Common/GraphicsCommon.hpp Common/TextureView.cpp Common/TextureView.hpp Common/TextureMemory.cpp Common/TextureMemory.hpp Common/Enums.hpp Common/EnumHelpers.cpp Common/EnumHelpers.hpp)

if (GraphicsApi STREQUAL "VK")
    set(SrcFiles ${CommonFiles} ${VkFiles})
endif()

add_library(KryneEngineGraphics STATIC ${SrcFiles})
target_link_libraries(KryneEngineGraphics EASTL glfw)
target_include_directories(KryneEngineGraphics PUBLIC ${RootDir})

if (GraphicsApi STREQUAL "VK")
    message(STATUS "Finding and linking Vulkan lib")
    find_package(Vulkan REQUIRED FATAL_ERROR)

    message(STATUS "Vulkan include dirs: " ${Vulkan_INCLUDE_DIRS})
    target_include_directories(KryneEngineGraphics PUBLIC ${Vulkan_INCLUDE_DIRS})

    message(STATUS "Vulkan lib: " ${Vulkan_LIBRARIES})
    target_link_libraries(KryneEngineGraphics ${Vulkan_LIBRARIES})
endif()