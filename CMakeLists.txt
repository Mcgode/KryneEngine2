cmake_minimum_required(VERSION 3.20)
project(KryneEngine2 LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 23)

include(./CMake/BuildSystem.cmake)
include(./CMake/ShaderCompilation.cmake)

add_subdirectory(External)
add_subdirectory(Core)
add_subdirectory(Modules)
add_subdirectory(Examples)

enable_testing()
add_subdirectory(Tests)