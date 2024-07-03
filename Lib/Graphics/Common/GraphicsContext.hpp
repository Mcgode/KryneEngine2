/**
 * @file
 * @author Max Godefroy
 * @date 19/03/2022.
 */

#pragma once

#if defined(KE_GRAPHICS_API_VK)
#   include <Graphics/VK/VkGraphicsContext.hpp>
#elif defined(KE_GRAPHICS_API_DX12)
#   include <Graphics/DX12/Dx12GraphicsContext.hpp>
#else
#   error No valid graphics API
#endif

namespace KryneEngine
{
    class Window;

    class GraphicsContext
    {
    public:
        explicit GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo);

        ~GraphicsContext();

        [[nodiscard]] Window* GetWindow() const { return m_implementation.GetWindow(); }

        [[nodiscard]] u8 GetFrameContextCount() const { return m_implementation.GetFrameContextCount(); }

        bool EndFrame();

        void WaitForLastFrame() const;

        [[nodiscard]] const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const { return m_implementation.GetApplicationInfo(); }

    private:
#if defined(KE_GRAPHICS_API_VK)
        VkGraphicsContext m_implementation;
#elif defined(KE_GRAPHICS_API_DX12)
        Dx12GraphicsContext m_implementation;
#endif

        u64 m_frameId = 1;

    public:
        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc);
    };
}


