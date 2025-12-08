add_library(KryneEngine_Modules_GuiLib STATIC
        Src/ClayImpl.cpp
        Src/Context.cpp
        Include/KryneEngine/Modules/GuiLib/Context.hpp
        Include/KryneEngine/Modules/GuiLib/IGuiRenderer.hpp
        Include/KryneEngine/Modules/GuiLib/TextureRegion.hpp
)

target_include_directories(KryneEngine_Modules_GuiLib PUBLIC Include Extern/Clay)

target_link_libraries(KryneEngine_Modules_GuiLib PUBLIC KryneEngine_Core)