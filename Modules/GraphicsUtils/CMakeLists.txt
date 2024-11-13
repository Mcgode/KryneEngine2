file(GLOB GraphicsUtilsModuleSrc *.hpp *.cpp)

add_library(KryneEngine_Modules_GraphicsUtils ${GraphicsUtilsModuleSrc})
target_link_libraries(KryneEngine_Modules_GraphicsUtils KryneEngine_Core)
