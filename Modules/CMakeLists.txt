add_library(KryneEngine_Modules_Includes INTERFACE)
target_include_directories(KryneEngine_Modules_Includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(ShaderLib)
add_subdirectory(GraphicsUtils)
add_subdirectory(ImGui)
add_subdirectory(RenderGraph)

add_library(KryneEngine_Modules INTERFACE)
target_link_libraries(KryneEngine_Modules INTERFACE
        KryneEngine_Modules_GraphicsUtils
        KryneEngine_Modules_ImGui
        KryneEngine_Modules_RenderGraph)
target_include_directories(KryneEngine_Modules INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(KryneEngine INTERFACE)
target_link_libraries(KryneEngine INTERFACE KryneEngineCore KryneEngine_Modules)