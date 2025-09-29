add_subdirectory(ShaderLib)
add_subdirectory(GraphicsUtils)
add_subdirectory(ImGui)
add_subdirectory(RenderGraph)
add_subdirectory(RenderGraphDebug)
add_subdirectory(ShaderReflection)
add_subdirectory(SdfTexture)

add_library(KryneEngine_Modules INTERFACE)
target_link_libraries(KryneEngine_Modules INTERFACE
        KryneEngine_Modules_GraphicsUtils
        KryneEngine_Modules_ImGui
        KryneEngine_Modules_RenderGraph
        KryneEngine_Modules_RenderGraphDebug
        KryneEngine_Modules_ShaderReflection
        KryneEngine_Modules_SdfTexture
)

add_library(KryneEngine INTERFACE)
target_link_libraries(KryneEngine INTERFACE KryneEngine_Core KryneEngine_Modules)