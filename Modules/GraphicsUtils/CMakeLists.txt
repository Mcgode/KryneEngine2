add_library(KryneEngine_Modules_GraphicsUtils
        Include/KryneEngine/Modules/GraphicsUtils/DynamicBuffer.hpp
        Src/DynamicBuffer.cpp
)

target_link_libraries(KryneEngine_Modules_GraphicsUtils KryneEngine_Core)

target_include_directories(KryneEngine_Modules_GraphicsUtils PUBLIC Include)
target_include_directories(KryneEngine_Modules_GraphicsUtils PRIVATE Src)
