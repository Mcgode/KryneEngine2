set(RootDir ${CMAKE_SOURCE_DIR}/Core)
add_subdirectory(Graphics)

set(CommonSrc
        Common/additional_impl.cpp 
        Common/Assert.hpp Common/Assert.cpp
        Common/BitUtils.hpp
        Common/EastlHelpers.hpp
        Common/StringHelpers.hpp
        Common/Types.hpp
        Common/Utils/MultiFrameTracking.hpp
        Common/Utils/Alignment.hpp
)

set(FilesSrc
        Files/FileWatcher.cpp Files/FileWatcher.hpp
        Files/WatchedDirectory.cpp Files/WatchedDirectory.hpp
        Files/FileSystemHelper.cpp Files/FileSystemHelper.hpp
        Files/IoQueryManager.cpp Files/IoQueryManager.hpp
        Files/File.cpp Files/File.hpp
        )

set(MemorySrc
        Memory/RangeMapping.hpp
        Memory/GenerationalPool.inl Memory/GenerationalPool.hpp
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
        Threads/Semaphore.hpp
        )

add_library(KryneEngineCore ${CommonSrc} ${FilesSrc} ${MemorySrc} ${ThreadsSrc})
target_link_libraries(KryneEngineCore KryneEngineGraphics moodycamel)