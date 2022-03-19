set(RootDir ${CMAKE_SOURCE_DIR}/Lib)
add_subdirectory(Graphics)

add_library(KryneEngine Common/additional_impl.cpp)
target_link_libraries(KryneEngine KryneEngineGraphics)