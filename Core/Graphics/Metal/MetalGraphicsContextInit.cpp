/**
 * @file
 * @author Max Godefroy
 * @date 28/10/2024.
 */

#include "MetalGraphicsContext.hpp"

#include <Graphics/Metal/MetalFrameContext.hpp>
#include <Graphics/Metal/MetalSwapChain.hpp>
#include <Window/Window.hpp>

namespace KryneEngine
{
    MetalGraphicsContext::MetalGraphicsContext(
            const GraphicsCommon::ApplicationInfo& _appInfo,
            const Window* _window,
            u64 _initialFrameId)
                : m_applicationInfo(_appInfo)
    {
        m_device = MTL::CreateSystemDefaultDevice();

        if (_appInfo.m_features.m_graphics)
        {
            // Catch internal auto release
            KE_AUTO_RELEASE_POOL;
            m_graphicsQueue = m_device->newCommandQueue();
        }

        if (_appInfo.m_features.m_compute)
        {
            if (_appInfo.m_features.m_asyncCompute || m_graphicsQueue == nullptr)
            {
                // Catch internal auto release
                KE_AUTO_RELEASE_POOL;
                m_computeQueue = m_device->newCommandQueue();
            }
        }

        if (_appInfo.m_features.m_transfer)
        {
            if (_appInfo.m_features.m_transferQueue || (m_computeQueue == nullptr && m_graphicsQueue == nullptr))
            {
                NsPtr<MTL::IOCommandQueueDescriptor> descriptor { MTL::IOCommandQueueDescriptor::alloc()->init() };

                // Catch internal auto release
                KE_AUTO_RELEASE_POOL;
                m_ioQueue = m_device->newIOCommandQueue(descriptor.get(), nullptr);
            }
        }

        m_frameContextCount = 2;
        const u8 frameIndex = _initialFrameId % m_frameContextCount;

        KE_ASSERT_FATAL_MSG(!_appInfo.m_features.m_present || _appInfo.m_features.m_graphics,
                            "Metal graphics context does not support presentation without graphics queue");
        KE_ASSERT_FATAL(!(_appInfo.m_features.m_present ^ (_window != nullptr)));
        if (_appInfo.m_features.m_present)
        {
            m_swapChain = eastl::make_unique<MetalSwapChain>(*m_device, _appInfo, _window, m_resources, frameIndex);
            m_frameContextCount = m_swapChain->m_textures.Size();
        }

        m_frameContexts.Resize(m_frameContextCount);
        m_frameContexts.InitAll(
            m_graphicsQueue != nullptr,
            m_computeQueue != nullptr,
            m_ioQueue != nullptr);

        m_frameContexts[frameIndex].PrepareForNextFrame(_initialFrameId);

        m_argumentBufferManager.Init(m_frameContextCount, frameIndex);
    }

    MetalGraphicsContext::~MetalGraphicsContext() = default;
}