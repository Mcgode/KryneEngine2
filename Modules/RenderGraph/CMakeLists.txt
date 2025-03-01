
add_library(KryneEngine_Modules_RenderGraph STATIC
        Src/Builder.cpp
        Include/KryneEngine/Modules/RenderGraph/Builder.hpp
        Include/KryneEngine/Modules/RenderGraph/Declarations/PassDeclaration.hpp
        Src/Declarations/PassAttachmentDeclaration.cpp
        Include/KryneEngine/Modules/RenderGraph/Declarations/PassAttachmentDeclaration.hpp
        Src/Declarations/PassDeclaration.cpp
        Include/KryneEngine/Modules/RenderGraph/Utils/DeclarationBuilder.hpp
        Src/Registry.cpp
        Include/KryneEngine/Modules/RenderGraph/Registry.hpp
        Include/KryneEngine/Modules/RenderGraph/Resource.hpp
        Src/RenderGraph.cpp
        Include/KryneEngine/Modules/RenderGraph/RenderGraph.hpp
)

target_link_libraries(KryneEngine_Modules_RenderGraph KryneEngine_Core)

target_compile_definitions(KryneEngine_Modules_RenderGraph PRIVATE KE_TRACY_COLOR=0xe89005 KE_TRACY_FUNC_COLOR=Oxec7505)

target_include_directories(KryneEngine_Modules_RenderGraph PUBLIC Include)
target_include_directories(KryneEngine_Modules_RenderGraph PRIVATE Src)