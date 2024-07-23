file(GLOB ImGuiModuleSrc *.hpp *.cpp)

add_library(ImGuiModule ${ImGuiModuleSrc})
target_link_libraries(ImGuiModule KryneEngineCore DearImgui)