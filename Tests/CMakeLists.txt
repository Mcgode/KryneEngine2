add_library(TestUtils
        Utils/AssertUtils.hpp
        Utils/AssertUtils.cpp
        Utils/Comparison.hpp)

target_include_directories(TestUtils PUBLIC .)

target_link_libraries(TestUtils KryneEngineCore gtest)

add_subdirectory(Core)