
# Set up HelloTriangle executable.
add_executable(HelloTriangle HelloTriangle.cpp)

# Link to KryneEngine (Core lib only).
target_link_libraries(HelloTriangle KryneEngine_Core)

# Link shader lib and compile shaders.
target_link_shader_libraries(HelloTriangle ShaderLib)
target_compile_shaders(HelloTriangle Shaders Examples/HelloTriangle)

# Add shader dir symlink for easy working dir access to shaders.
target_shaders_dir_symlink(HelloTriangle)
