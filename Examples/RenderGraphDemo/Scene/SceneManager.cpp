/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2025.
 */

#include "SceneManager.hpp"

#include <KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>

#include "TorusKnot.hpp"

namespace KryneEngine::Samples::RenderGraphDemo
{
    SceneManager::SceneManager(AllocatorInstance _allocator, GraphicsContext* _graphicsContext)
        : m_allocator(_allocator)
        , m_torusKnot(nullptr, m_allocator)
    {
        m_torusKnot.reset(m_allocator.New<TorusKnot>(m_allocator));
    }

    SceneManager::~SceneManager() = default;

    void SceneManager::Process(GraphicsContext* _graphicsContext)
    {
        m_torusKnot->Process(_graphicsContext);
    }

    void SceneManager::ExecuteTransfers(GraphicsContext* _graphicsContext, CommandListHandle _commandList)
    {
        KE_ZoneScopedFunction("SceneManager::ExecuteTransfers");

        m_torusKnot->ProcessTransfers(_graphicsContext, _commandList);
    }
}