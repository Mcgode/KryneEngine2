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

        [[nodiscard]] inline Window* GetWindow() const
        {
            return m_implementation.GetWindow();
        }

        [[nodiscard]] inline u8 GetFrameContextCount() const
        {
            return m_implementation.GetFrameContextCount();
        }

        bool EndFrame();

        void WaitForLastFrame() const;

        [[nodiscard]] inline const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const
        {
            return m_implementation.GetApplicationInfo();
        }

    private:
#if defined(KE_GRAPHICS_API_VK)
        using UnderlyingGraphicsContext = VkGraphicsContext;
#elif defined(KE_GRAPHICS_API_DX12)
        using UnderlyingGraphicsContext = Dx12GraphicsContext;
#endif
        UnderlyingGraphicsContext m_implementation;

        u64 m_frameId = 1;

    public:
        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc)
        {
            return m_implementation.CreateRenderTargetView(_desc);
        }

        bool DestroyRenderTargetView(GenPool::Handle _handle)
        {
            return m_implementation.DestroyRenderTargetView(_handle);
        }

        [[nodiscard]] GenPool::Handle CreateRenderPass(const RenderPassDesc& _desc)
        {
            return m_implementation.CreateRenderPass(_desc);
        }

        bool DestroyRenderPass(GenPool::Handle _handle)
        {
            return m_implementation.DestroyRenderPass(_handle);
        }

        void BeginRenderPass(CommandList _commandList, GenPool::Handle _handle)
        {
            m_implementation.BeginRenderPass(_commandList, _handle);
        }

        void EndRenderPass(CommandList _commandList)
        {
            m_implementation.EndRenderPass(_commandList);
        }
    };
}


