set(VkFiles
        VK/VkGraphicsContext.hpp VK/VkGraphicsContext.cpp
        VK/HelperFunctions.hpp
        VK/VkSwapChain.cpp VK/VkSwapChain.hpp
        VK/VkSurface.cpp VK/VkSurface.hpp
        VK/CommonStructures.hpp
        VK/VkHeaders.hpp
        VK/VkFrameContext.cpp VK/VkFrameContext.hpp
        VK/VkResources.cpp VK/VkResources.hpp
        VK/VkTypes.hpp
)

set (DX12Files
        DX12/Dx12GraphicsContext.cpp DX12/Dx12GraphicsContext.hpp
        DX12/Dx12Headers.hpp
        DX12/HelperFunction.cpp DX12/HelperFunctions.hpp
        DX12/Dx12SwapChain.cpp DX12/Dx12SwapChain.hpp
        DX12/Dx12FrameContext.cpp DX12/Dx12FrameContext.hpp
        DX12/Dx12Resources.cpp
        DX12/Dx12Resources.h
        DX12/Dx12Types.hpp
)

set(CommonFiles
        Common/GraphicsContext.cpp Common/GraphicsContext.hpp
        Common/Window.cpp Common/Window.hpp
        Common/GraphicsCommon.hpp
        Common/Texture.hpp
        Common/Enums.hpp
        Common/EnumHelpers.cpp Common/EnumHelpers.hpp
        Common/RenderTargetView.hpp
        Common/RenderPass.hpp
)

if (GraphicsApi STREQUAL "VK")
    set(SrcFiles ${CommonFiles} ${VkFiles})
    set(PrecompileHeaders VK/VkHeaders.hpp)
elseif(GraphicsApi STREQUAL "DX12")
    set(SrcFiles ${CommonFiles} ${DX12Files})
    set(PrecompileHeaders DX12/Dx12Headers.hpp)
endif()

add_library(KryneEngineGraphics STATIC ${SrcFiles})
target_link_libraries(KryneEngineGraphics EASTL glfw glm)
target_include_directories(KryneEngineGraphics PUBLIC ${RootDir})

if (GraphicsApi STREQUAL "VK")
    message(STATUS "Finding and linking Vulkan lib")
    find_package(Vulkan REQUIRED FATAL_ERROR)

    message(STATUS "Vulkan include dirs: " ${Vulkan_INCLUDE_DIRS})
    target_include_directories(KryneEngineGraphics PUBLIC ${Vulkan_INCLUDE_DIRS})

    message(STATUS "Vulkan lib: " ${Vulkan_LIBRARIES})
    target_link_libraries(KryneEngineGraphics ${Vulkan_LIBRARIES})
elseif(GraphicsApi STREQUAL "DX12")
    message(STATUS "Linking DirectX12 lib")
    target_link_libraries(KryneEngineGraphics d3d12 dxgi d3dcompiler dxguid DirectX-Headers)
endif()

if (PrecompileHeaders)
    message(STATUS "Precompiling indicated headers: ${PrecompileHeaders}")
    target_precompile_headers(KryneEngineGraphics PUBLIC ${PrecompileHeaders})
endif()