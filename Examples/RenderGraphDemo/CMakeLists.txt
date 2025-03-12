
# Set up Render graph demo executable
add_executable(RenderGraphDemo
    RenderGraphDemo.cpp
        Scene/TorusKnotMeshGenerator.cpp
        Scene/TorusKnotMeshGenerator.hpp
)

# Link to cpp libraries
target_link_libraries(RenderGraphDemo
        KryneEngine_Modules_RenderGraph
        KryneEngine_Modules_ImGui
)

# Link shader libraries and compile sample shaders
target_link_shader_libraries(RenderGraphDemo ShaderLib)
target_compile_shaders(RenderGraphDemo Shaders Examples/RenderGraphDemo)

# Symlink compiled shaders directory
target_shaders_dir_symlink(RenderGraphDemo)