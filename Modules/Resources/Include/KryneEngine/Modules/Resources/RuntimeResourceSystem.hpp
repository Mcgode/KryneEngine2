/**
 * @file
 * @author Max Godefroy
 * @date 16/01/2026.
 */

#pragma once

#include <EASTL/hash_map.h>
#include <KryneEngine/Core/Threads/RwSpinLock.hpp>

#include "KryneEngine/Modules/Resources/ResourceTypeId.hpp"

namespace KryneEngine::Modules::Resources
{
    class IResourceManager;

    class RuntimeResourceSystem
    {
    public:
        explicit RuntimeResourceSystem(AllocatorInstance _allocator);

        template <class Resource>
        void RegisterResourceManager(IResourceManager* _resourceManager)
        {
            RegisterResourceManager(_resourceManager, Resource::kTypeId);
        }

        template <class Resource, class ResourceManager>
        ResourceManager* GetResourceManager()
        {
            return reinterpret_cast<ResourceManager*>(GetResourceManager(Resource::kTypeId));
        }

    private:
        AllocatorInstance m_allocator;

        eastl::hash_map<ResourceTypeId, IResourceManager*> m_resourceManagers;
        alignas(Threads::kCacheLineSize) mutable RwSpinLock m_resourceManagersLock {};

        void RegisterResourceManager(IResourceManager* _resourceManager, ResourceTypeId _typeId);
        IResourceManager* GetResourceManager(ResourceTypeId _typeId) const;
    };
}
