/**
 * @file
 * @author Max Godefroy
 * @date 11/03/2023.
 */

#pragma once

#include "Dx12Headers.hpp"
#include "Dx12FrameContext.hpp"
#include "Dx12Resources.h"
#include "Dx12Types.hpp"
#include <Common/Arrays.hpp>
#include <Graphics/Common/MemoryBarriers.hpp>
#include <Graphics/Common/Texture.hpp>
#include <EASTL/unique_ptr.h>

namespace KryneEngine
{
    class Window;
    class Dx12SwapChain;

    struct BufferMapping;

    class Dx12GraphicsContext
    {
    public:
        explicit Dx12GraphicsContext(const GraphicsCommon::ApplicationInfo &_appInfo, u64 _currentFrameId);

        ~Dx12GraphicsContext();

        [[nodiscard]] Window* GetWindow() const;

        [[nodiscard]] u8 GetFrameContextCount() const { return m_frameContextCount; }

        void EndFrame(u64 _frameId);

        [[nodiscard]] bool IsFrameExecuted(u64 _frameId) const;

        void WaitForFrame(u64 _frameId) const;

        [[nodiscard]] const GraphicsCommon::ApplicationInfo& GetApplicationInfo() const { return m_appInfo; }

    private:
        GraphicsCommon::ApplicationInfo m_appInfo;

        eastl::unique_ptr<Window> m_window;

        ComPtr<ID3D12Device> m_device;

        ComPtr<ID3D12CommandQueue> m_directQueue;
        ComPtr<ID3D12CommandQueue> m_computeQueue;
        ComPtr<ID3D12CommandQueue> m_copyQueue;

        eastl::unique_ptr<Dx12SwapChain> m_swapChain;

        u8 m_frameContextCount;
        DynamicArray<Dx12FrameContext> m_frameContexts;
        ComPtr<ID3D12Fence> m_frameFence;
        HANDLE m_frameFenceEvent;

        DWORD m_validationLayerMessageCallbackHandle = 0;

        bool m_enhancedBarriersEnabled = false;

        void _CreateDevice(IDXGIFactory4* _factory4);
        void _FindAdapter(IDXGIFactory4* _factory, IDXGIAdapter1** _adapter);

        void _CreateCommandQueues();

    public:
        [[nodiscard]] eastl::vector<TextureMemoryFootprint> FetchTextureSubResourcesMemoryFootprints(const TextureDesc& _desc);

        [[nodiscard]] inline GenPool::Handle CreateBuffer(const BufferCreateDesc& _desc)
        {
            return m_resources.CreateBuffer(_desc);
        }

        [[nodiscard]] inline GenPool::Handle CreateStagingBuffer(
            const TextureDesc& _createDesc,
            const eastl::vector<TextureMemoryFootprint>& _footprints)
        {
            return m_resources.CreateStagingBuffer(_createDesc, _footprints);
        }

        inline bool DestroyBuffer(GenPool::Handle _bufferHandle)
        {
            return m_resources.DestroyBuffer(_bufferHandle);
        }

        [[nodiscard]] GenPool::Handle CreateTexture(const TextureCreateDesc& _createDesc)
        {
            return m_resources.CreateTexture(_createDesc, m_device.Get());
        }

        inline bool DestroyTexture(GenPool::Handle _handle)
        {
            return m_resources.ReleaseTexture(_handle, true);
        }

        [[nodiscard]] inline GenPool::Handle CreateTextureSrv(const TextureSrvDesc& _srvDesc, u64 _frameId)
        {
            return m_resources.CreateTextureSrv(_srvDesc, m_device.Get(), _frameId % GetFrameContextCount());
        }

        inline bool DestroyTextureSrv(GenPool::Handle _handle)
        {
            return m_resources.DestroyTextureSrv(_handle);
        }

        [[nodiscard]] inline GenPool::Handle CreateRenderTargetView(const RenderTargetViewDesc& _desc)
        {
            return m_resources.CreateRenderTargetView(_desc, m_device.Get());
        }

        bool DestroyRenderTargetView(GenPool::Handle _handle)
        {
            return m_resources.FreeRenderTargetView(_handle);
        }

        [[nodiscard]] GenPool::Handle GetPresentRenderTarget(u8 _index);
        [[nodiscard]] u32 GetCurrentPresentImageIndex() const;

        GenPool::Handle CreateRenderPass(const RenderPassDesc& _desc)
        {
            return m_resources.CreateRenderPass(_desc);
        }

        bool DestroyRenderPass(GenPool::Handle _handle)
        {
            return m_resources.FreeRenderPass(_handle);
        }

        CommandList BeginGraphicsCommandList(u64 _frameId);
        void EndGraphicsCommandList(u64 _frameId);

        void BeginRenderPass(CommandList _commandList, GenPool::Handle _handle);
        void EndRenderPass(CommandList _commandList);

        void SetTextureData(
            CommandList _commandList,
            GenPool::Handle _stagingBuffer,
            GenPool::Handle _dstTexture,
            const TextureMemoryFootprint& _footprint,
            const SubResourceIndexing& _subResourceIndex,
            void* _data);

        void MapBuffer(BufferMapping& _mapping);
        void UnmapBuffer(BufferMapping& _mapping);

        void PlaceMemoryBarriers(
            CommandList _commandList,
            const eastl::span<GlobalMemoryBarrier>& _globalMemoryBarriers,
            const eastl::span<BufferMemoryBarrier>& _bufferMemoryBarriers,
            const eastl::span<TextureMemoryBarrier>& _textureMemoryBarriers);

    private:
        Dx12Resources m_resources;
        GenPool::Handle m_currentRenderPass;
    };
} // KryneEngine