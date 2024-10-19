cmake_minimum_required(VERSION 3.20)

add_executable(Core_Graphics_UnitTests
        GraphicContext/Common.h
        GraphicContext/GraphicsContext_UnitTests.cpp
        GraphicContext/Buffer_UnitTests.cpp
        GraphicContext/Texture_UnitTests.cpp
)

target_link_libraries(Core_Graphics_UnitTests KryneEngineCore KryneEngineGraphics TestUtils gtest gtest_main)

add_test(NAME Core_Graphics_UnitTests COMMAND Core_Graphics_UnitTests)