/**
 * @file
 * @author Max Godefroy
 * @date 10/03/2025.
 */

#pragma once

#include <EASTL/optional.h>
#include <KryneEngine/Core/Memory/SimplePool.hpp>
#include <KryneEngine/Core/Graphics/Common/MemoryBarriers.hpp>


namespace KryneEngine::Modules::RenderGraph
{
    struct Dependency
    {
        SimplePoolHandle m_resource;
        BarrierSyncStageFlags m_targetSyncStage = BarrierSyncStageFlags::All;
        eastl::optional<BarrierSyncStageFlags> m_finalSyncStage {};
        BarrierAccessFlags m_targetAccessFlags = BarrierAccessFlags::None;
        eastl::optional<BarrierAccessFlags> m_finalAccessFlags {};
        TextureLayout m_targetLayout = TextureLayout::Unknown;
        eastl::optional<TextureLayout> m_finalLayout {};
    };
}