/**
 * @file
 * @author Max Godefroy
 * @date 27/09/2025.
 */

#pragma once

#include <KryneEngine/Modules/RenderGraph/Builder.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    class ImGuiDebugWindow
    {
    public:
        static void DebugBuilder(
            const Builder& _builder,
            const Registry& _registry,
            AllocatorInstance _tempAllocator,
            bool* _windowOpen = nullptr);

    private:
        static void DisplayBuilderPasses(
            const Builder& _builder,
            const Registry& _registry,
            AllocatorInstance _tempAllocator);
        static void DisplayBuilderResources(
            const Builder& _builder,
            const Registry& _registry,
            AllocatorInstance _tempAllocator);
    };
}