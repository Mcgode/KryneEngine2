file(GLOB ImGuiModuleSrc *.hpp *.cpp)

add_library(ImGuiModule ${ImGuiModuleSrc})
target_link_shader_libraries(ImGuiModule ShaderLib ShaderLib)
target_compile_shaders(ImGuiModule Shaders Modules/ImGui)
target_link_libraries(ImGuiModule KryneEngineCore KEModuleIncludes DearImgui GraphicsUtilsModule)
