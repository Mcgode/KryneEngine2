cmake_minimum_required(VERSION 3.20)

add_executable(Core_Common_UnitTests
        Assert_UnitTests.cpp
        Utils/Alignment_UnitTests.cpp
        Arrays_UnitTests.cpp)

target_link_libraries(Core_Common_UnitTests KryneEngineCore TestUtils gtest gtest_main)

add_test(NAME Core_Common_UnitTests COMMAND Core_Common_UnitTests)