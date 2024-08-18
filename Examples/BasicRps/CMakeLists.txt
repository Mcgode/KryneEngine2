add_library(BasicRpsRpsl STATIC
        hello_triangle.rpsl.g.c)

add_executable(BasicRps main.cpp)
target_shaders_dir_symlink(BasicRps)
target_link_libraries(BasicRps KryneEngine BasicRpsRpsl)