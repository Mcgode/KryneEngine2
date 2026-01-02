add_executable(UiDemo UiDemo.cpp
        TextureGenerator.cpp
        TextureGenerator.hpp)

target_link_libraries(UiDemo KryneEngine_Modules_GuiLib)

target_shaders_dir_symlink(UiDemo)