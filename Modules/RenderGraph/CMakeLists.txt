
add_library(KryneEngine_Modules_RenderGraph STATIC
        Builder.cpp
        Builder.hpp
        PassDeclaration.hpp
)

target_link_libraries(KryneEngine_Modules_RenderGraph KryneEngine_Core KryneEngine_Modules_Includes)