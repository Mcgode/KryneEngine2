set(RootDir ${CMAKE_SOURCE_DIR}/Lib)
add_subdirectory(Graphics)

set(CommonSrc
        Common/additional_impl.cpp Common/Arrays.hpp)

set(ThreadsSrc
        Threads/FibersManager.cpp Threads/FibersManager.hpp Threads/FiberThread.cpp Threads/FiberThread.hpp Threads/HelperFunctions.cpp Threads/HelperFunctions.hpp)

add_library(KryneEngine ${CommonSrc} ${ThreadsSrc})
target_link_libraries(KryneEngine KryneEngineGraphics moodycamel)