add_library(KryneEngine_Modules_ShaderReflection STATIC
        Src/Blob.cpp
        Include/KryneEngine/Modules/ShaderReflection/Blob.hpp
)

target_include_directories(KryneEngine_Modules_ShaderReflection PUBLIC Include)

target_link_libraries(KryneEngine_Modules_ShaderReflection PUBLIC KryneEngine_Core)