/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#include "SceneManager.hpp"

#include <KryneEngine/Core/Profiling/TracyHeader.hpp>

namespace KryneEngine::Samples::RenderGraphDemo
{
    SceneManager::SceneManager(AllocatorInstance _allocator)
        : m_allocator(_allocator)
    {}

    SceneManager::~SceneManager() = default;

    void SceneManager::ExecuteTransfers(GraphicsContext* _graphicsContext, CommandListHandle _commandList)
    {
        KE_ZoneScopedFunction("SceneManager::ExecuteTransfers");
    }
}