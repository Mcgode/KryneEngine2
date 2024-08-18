file(GLOB_RECURSE RpsSrc *.hpp *.cpp)

add_library(RpsRuntimeModule ${RpsSrc})
target_link_libraries(RpsRuntimeModule KryneEngineCore rps_runtime)