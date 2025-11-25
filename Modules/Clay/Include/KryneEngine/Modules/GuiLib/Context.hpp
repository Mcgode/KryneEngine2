/**
 * @file
 * @author Max Godefroy
 * @date 06/07/2025.
 */

#pragma once

#include <clay.h>
#include <KryneEngine/Core/Common/Utils/Macros.hpp>
#include <KryneEngine/Core/Graphics/GraphicsContext.hpp>
#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>

namespace KryneEngine::Modules::Clay
{
    class Context
    {
    public:
        explicit Context(AllocatorInstance _allocator);

        KE_DEFINE_COPY_MOVE_SEMANTICS(Context, delete, delete);

        void Initialize(GraphicsContext& _graphicsContext);

        void Destroy(GraphicsContext& _graphicsContext);

    private:
        AllocatorInstance m_allocator;
        char* m_arenaMemory = nullptr;
        Clay_Context* m_clayContext = nullptr;

        static void ErrorHandler(Clay_ErrorData _errorData);
    };
} // namespace KryneEngine::Modules::Clay

