add_executable(Basic main.cpp BasicRps.hpp BasicRps.cpp)
target_shaders_dir_symlink(Basic)
target_link_libraries(Basic KryneEngine)