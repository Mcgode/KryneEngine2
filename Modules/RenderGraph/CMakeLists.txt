
add_library(KryneEngine_Modules_RenderGraph STATIC
        Builder.cpp
        Builder.hpp
        Declarations/PassDeclaration.hpp
        Declarations/PassAttachmentDeclaration.cpp
        Declarations/PassAttachmentDeclaration.hpp
        Declarations/PassDeclaration.cpp
        Utils/DeclarationBuilder.hpp
        Registry.cpp
        Registry.hpp
        Resource.hpp
)

target_link_libraries(KryneEngine_Modules_RenderGraph KryneEngine_Core KryneEngine_Modules_Includes)