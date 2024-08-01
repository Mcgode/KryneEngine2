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

        [[nodiscard]] inline u64 GetFrameId() const
        {
            return m_frameId;
        }

        [[nodiscard]] inline u8 GetFrameContextCount() const
        {
            return m_implementation.GetFrameContextCount();
        }

        [[nodiscard]] inline u8 GetCurrentFrameContextIndex() const
        {
            return m_frameId % GetFrameContextCount();
        }

        bool EndFrame();

        void WaitForLastFrame() const;

        [[nodiscard]] inline bool IsFrameExecuted(u64 _frameId) const
        {
            return m_implementation.IsFrameExecuted(_frameId);
        }

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

        static constexpr u64 kInitialFrameId = 1;
        u64 m_frameId;

    public:
        [[nodiscard]] eastl::vector<TextureMemoryFootprint> FetchTextureSubResourcesMemoryFootprints(const TextureDesc& _desc)
        {
            return m_implementation.FetchTextureSubResourcesMemoryFootprints(_desc);
        }

        [[nodiscard]] inline GenPool::Handle CreateTexture(const TextureCreateDesc& _createDesc)
        {
            if (!KE_VERIFY_MSG(
                    BitUtils::EnumHasAll(_createDesc.m_memoryUsage, MemoryUsage::GpuOnly_UsageType),
                    "The engine is designed around having buffers representing textures on the CPU")) [[unlikely]]
            {
                return GenPool::kInvalidHandle;
            }

            return m_implementation.CreateTexture(_createDesc);
        }

        inline bool DestroyTexture(GenPool::Handle _handle)
        {
            return m_implementation.DestroyTexture(_handle);
        }

        [[nodiscard]] GenPool::Handle CreateTextureSrv(const TextureSrvDesc& _srvDesc)
        {
            return m_implementation.CreateTextureSrv(_srvDesc, m_frameId);
        }

        [[nodiscard]] GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc)
        {
            return m_implementation.CreateRenderTargetView(_desc);
        }

        bool DestroyRenderTargetView(GenPool::Handle _handle)
        {
            return m_implementation.DestroyRenderTargetView(_handle);
        }

        [[nodiscard]] GenPool::Handle GetPresentRenderTarget(u8 _swapChainIndex)
        {
            return m_implementation.GetPresentRenderTarget(_swapChainIndex);
        }

        [[nodiscard]] inline u32 GetCurrentPresentImageIndex() const
        {
            return m_implementation.GetCurrentPresentImageIndex();
        }

        [[nodiscard]] GenPool::Handle CreateRenderPass(const RenderPassDesc& _desc)
        {
            return m_implementation.CreateRenderPass(_desc);
        }

        bool DestroyRenderPass(GenPool::Handle _handle)
        {
            return m_implementation.DestroyRenderPass(_handle);
        }

        CommandList BeginGraphicsCommandList()
        {
            return m_implementation.BeginGraphicsCommandList(m_frameId);
        }

        void EndGraphicsCommandList()
        {
            m_implementation.EndGraphicsCommandList(m_frameId);
        }

        void BeginRenderPass(CommandList _commandList, GenPool::Handle _handle)
        {
            m_implementation.BeginRenderPass(_commandList, _handle);
        }

        void EndRenderPass(CommandList _commandList)
        {
            m_implementation.EndRenderPass(_commandList);
        }

        inline void SetTextureData(
            CommandList _commandList,
            GenPool::Handle _stagingTexture,
            GenPool::Handle _dstTexture,
            const TextureMemoryFootprint& _footprint,
            const SubResourceIndexing& _subResourceIndex,
            void* _data)
        {
            m_implementation.SetTextureData(
                _commandList,
                _stagingTexture,
                _dstTexture,
                _footprint,
                _subResourceIndex,
                _data);
        }
    };
}


