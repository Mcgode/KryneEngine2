file(GLOB ImGuiModuleSrc *.hpp *.cpp)

add_library(KryneEngine_Modules_ImGui STATIC ${ImGuiModuleSrc})

target_link_shader_libraries(KryneEngine_Modules_ImGui ShaderLib)
target_compile_shaders(KryneEngine_Modules_ImGui Shaders Modules/ImGui)

target_link_libraries(KryneEngine_Modules_ImGui
        KryneEngine_Core
        DearImgui
        KryneEngine_Modules_Includes
        KryneEngine_Modules_GraphicsUtils)
