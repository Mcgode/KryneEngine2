add_library(KryneEngine_Modules_Clay STATIC
        Src/ClayImpl.cpp
        Src/Context.cpp
        Include/KryneEngine/Modules/GuiLib/Context.hpp
)

target_include_directories(KryneEngine_Modules_Clay PUBLIC Include Extern/Clay)

target_link_libraries(KryneEngine_Modules_Clay PUBLIC KryneEngine_Core)