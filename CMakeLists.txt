cmake_minimum_required(VERSION 3.21)
project(KryneEngine2)

set(CMAKE_CXX_STANDARD 23)
include(./CMake/BuildSystem.cmake)

add_subdirectory(External)
add_subdirectory(Lib)
add_subdirectory(Examples)
