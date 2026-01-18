/**
 * @file
 * @author Max Godefroy
 * @date 18/01/2026.
 */

#pragma once

#include <KryneEngine/Core/Memory/Allocators/Allocator.hpp>

#include "KryneEngine/Modules/Resources/ResourceEntry.hpp"

namespace KryneEngine::Modules::Resources
{
    template <class ResourceManager>
    class ResourceBase
    {
    public:
        [[nodiscard]] AllocatorInstance GetAllocator() const { return m_allocator; }

        virtual ~ResourceBase() = default;

    protected:
        AllocatorInstance m_allocator;
        ResourceManager* m_resourceManager;

        ResourceBase(AllocatorInstance _allocator, ResourceManager* _resourceManager)
            : m_allocator(_allocator)
            , m_resourceManager(_resourceManager)
        {}
    };

    template <class ResourceManager>
    class RefCountedResourceBase: public ResourceBase<ResourceManager>
    {
    public:
        s32 m_refCount = 0;

        ~RefCountedResourceBase() override
        {
            m_entry->m_resource.store(nullptr, std::memory_order::release);
        }

    protected:
        ResourceEntry* m_entry;

        RefCountedResourceBase(AllocatorInstance _allocator, ResourceManager* _resourceManager, ResourceEntry* _entry)
            : ResourceBase<ResourceManager>(_allocator, _resourceManager)
            , m_entry(_entry)
        {}
    };
}
