cmake_minimum_required(VERSION 3.20)

add_executable(Core_Threads_UnitTests
        SpinLock_UnitTests.cpp
        Semaphore_UnitTests.cpp)

target_link_libraries(Core_Threads_UnitTests KryneEngineCore TestUtils gtest gtest_main)

add_test(NAME Core_Threads_UnitTests COMMAND Core_Threads_UnitTests)