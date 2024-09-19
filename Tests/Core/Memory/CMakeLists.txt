cmake_minimum_required(VERSION 3.20)

add_executable(Core_Memory_UnitTests
    GenerationalPool_UnitTests.cpp)

target_link_libraries(Core_Memory_UnitTests KryneEngineCore TestUtils gtest gtest_main)

add_test(NAME Core_Memory_UnitTests COMMAND Core_Memory_UnitTests)