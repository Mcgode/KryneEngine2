file(GLOB GraphicsUtilsModuleSrc *.hpp *.cpp)

add_library(GraphicsUtilsModule ${GraphicsUtilsModuleSrc})
target_link_libraries(GraphicsUtilsModule KryneEngineCore)
