cmake_minimum_required(VERSION 3.20)

add_executable(Core_Math_UnitTests
        Vector_UnitTests.cpp
        RotationConversion_UnitTests.cpp
)

target_link_libraries(Core_Math_UnitTests KryneEngine_Core TestUtils gtest gtest_main)

add_test(NAME Core_Math_UnitTests COMMAND Core_Math_UnitTests)