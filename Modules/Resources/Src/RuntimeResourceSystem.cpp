/**
 * @file
 * @author Max Godefroy
 * @date 16/01/2026.
 */

#include "KryneEngine/Modules/Resources/RuntimeResourceSystem.hpp"

#include "KryneEngine/Core/Common/Assert.hpp"

namespace KryneEngine::Modules::Resources
{
    RuntimeResourceSystem::RuntimeResourceSystem(AllocatorInstance _allocator)
        : m_allocator(_allocator)
        , m_resourceManagers(_allocator)
    {}

    void RuntimeResourceSystem::RegisterResourceManager(IResourceManager* _resourceManager, ResourceTypeId _typeId)
    {
        const auto lock = m_resourceManagersLock.AutoWriteLock();
        KE_ASSERT(m_resourceManagers.find(_typeId) == m_resourceManagers.end());
        m_resourceManagers.emplace(_typeId, _resourceManager);
    }

    IResourceManager* RuntimeResourceSystem::GetResourceManager(const ResourceTypeId _typeId) const
    {
        const auto lock = m_resourceManagersLock.AutoReadLock();
        const auto it = m_resourceManagers.find(_typeId);
        return it != m_resourceManagers.end() ? it->second : nullptr;
    }
} // namespace KryneEngine::Modules::Resources