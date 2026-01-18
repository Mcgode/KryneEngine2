/**
 * @file
 * @author Max Godefroy
 * @date 17/01/2026.
 */

#pragma once

#include "KryneEngine/Core/Common/Assert.hpp"


#include <KryneEngine/Core/Memory/IntrusivePtr.hpp>
#include <atomic>

namespace KryneEngine::Modules::Resources
{
    struct ResourceEntry
    {
        std::atomic<void*> m_resource = nullptr;
        std::atomic<size_t> m_version = 0;
        u64 m_typeId = 0;

        template <class Resource> requires IsAllocatorIntrusible<Resource> && (!IsRefCountIntrusible<Resource>)
        Resource* UseResource() const
        {
            KE_ASSERT(Resource::kTypeId == m_typeId);
            return static_cast<Resource*>(m_resource.load(std::memory_order::acquire));
        }

        template <class Resource> requires IsAllocatorIntrusible<Resource> && IsRefCountIntrusible<Resource>
        IntrusiveSharedPtr<Resource> UseResource() const
        {
            KE_ASSERT(Resource::kTypeId == m_typeId);
            return reinterpret_cast<IntrusiveSharedPtr<Resource>>(m_resource.load(std::memory_order::acquire));
        }
    };
}