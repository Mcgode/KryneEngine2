/**
 * @file
 * @author Max Godefroy
 * @date 06/07/2025.
 */

#include "KryneEngine/Modules/GuiLib/Context.hpp"

#include <clay.h>

namespace KryneEngine::Modules::GuiLib
{
    Context::Context(AllocatorInstance _allocator)
        : m_allocator(_allocator)
    {}

    void Context::Initialize(GraphicsContext& _graphicsContext)
    {
        const u32 arenaCapacity = Clay_MinMemorySize();
        m_arenaMemory = m_allocator.Allocate<char>(arenaCapacity);

        const Clay_Arena arena {
            .capacity = arenaCapacity,
            .memory = m_arenaMemory
        };

        const Clay_Dimensions dimensions {
            .width = static_cast<float>(_graphicsContext.GetApplicationInfo().m_displayOptions.m_width),
            .height = static_cast<float>(_graphicsContext.GetApplicationInfo().m_displayOptions.m_height),
        };

        const Clay_ErrorHandler errorHandler {
            .errorHandlerFunction = ErrorHandler,
            .userData = this,
        };

        Clay_Initialize(arena, dimensions, errorHandler);
        m_clayContext = Clay_GetCurrentContext();
    }

    void Context::Destroy(GraphicsContext& _graphicsContext)
    {
        Clay_SetCurrentContext(nullptr);
        m_clayContext = nullptr;
        m_allocator.deallocate(m_arenaMemory);
    }

    void ErrorHandler(Clay_ErrorData _errorData)
    {
        KE_ERROR(_errorData.errorText.chars);
    }
} // namespace KryneEngine::Modules::Clay

