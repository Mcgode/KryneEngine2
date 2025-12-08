/**
 * @file
 * @author Max Godefroy
 * @date 06/07/2025.
 */

#include "KryneEngine/Modules/GuiLib/Context.hpp"

#include "KryneEngine/Core/Memory/Containers/StableVector.inl"
#include "KryneEngine/Modules/GuiLib/IGuiRenderer.hpp"

namespace KryneEngine::Modules::GuiLib
{
    Context::Context(AllocatorInstance _allocator)
        : m_allocator(_allocator)
        , m_registeredRegions(_allocator)
    {}

    void Context::Initialize(IGuiRenderer* _renderer, const uint2& _viewportSize)
    {
        const u32 arenaCapacity = Clay_MinMemorySize();
        m_arenaMemory = m_allocator.Allocate<char>(arenaCapacity);
        m_renderer = _renderer;

        const Clay_Arena arena {
            .capacity = arenaCapacity,
            .memory = m_arenaMemory
        };

        const Clay_Dimensions dimensions {
            .width = static_cast<float>(_viewportSize.x),
            .height = static_cast<float>(_viewportSize.y),
        };

        const Clay_ErrorHandler errorHandler {
            .errorHandlerFunction = ErrorHandler,
            .userData = this,
        };

        Clay_Initialize(arena, dimensions, errorHandler);
        m_clayContext = Clay_GetCurrentContext();
    }

    void Context::Destroy()
    {
        Clay_SetCurrentContext(nullptr);
        m_clayContext = nullptr;
        m_allocator.deallocate(m_arenaMemory);
    }

    void* Context::RegisterTextureRegion(TextureRegion&& _region)
    {
        return &m_registeredRegions.PushBack(std::move(_region));
    }

    void Context::ErrorHandler(Clay_ErrorData _errorData)
    {
        KE_ERROR(_errorData.errorText.chars);
    }

    Context::~Context() = default;
} // namespace KryneEngine::Modules::Clay

