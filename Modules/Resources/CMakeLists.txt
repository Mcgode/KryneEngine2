add_library(KryneEngine_Modules_Resources
        Src/RuntimeResourceSystem.cpp
        Include/KryneEngine/Modules/Resources/RuntimeResourceSystem.hpp
        Include/KryneEngine/Modules/Resources/ResourceTypeId.hpp
)

target_include_directories(KryneEngine_Modules_Resources PUBLIC Include)

target_link_libraries(KryneEngine_Modules_Resources PUBLIC KryneEngine_Core)