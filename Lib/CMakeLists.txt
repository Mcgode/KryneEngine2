set(RootDir ${CMAKE_SOURCE_DIR}/Lib)
add_subdirectory(Graphics)

set(CommonSrc
        Common/additional_impl.cpp
        )

set(ThreadsSrc
        Threads/FibersManager.cpp Threads/FibersManager.hpp
        Threads/FiberThread.cpp Threads/FiberThread.hpp
        Threads/HelperFunctions.cpp Threads/HelperFunctions.hpp
        Threads/FiberTls.inl Threads/FiberTls.hpp
        Threads/FiberJob.cpp Threads/FiberJob.hpp
        Threads/Internal/FiberContext.cpp Threads/Internal/FiberContext.hpp
        Threads/SyncCounterPool.cpp Threads/SyncCounterPool.hpp
        Threads/LightweightBinarySemaphore.hpp
        Threads/LightweightMutex.hpp Threads/SpinLock.hpp
        Threads/RwMutex.hpp
        Threads/Semaphore.hpp)

add_library(KryneEngine ${CommonSrc} ${ThreadsSrc})
target_link_libraries(KryneEngine KryneEngineGraphics moodycamel)