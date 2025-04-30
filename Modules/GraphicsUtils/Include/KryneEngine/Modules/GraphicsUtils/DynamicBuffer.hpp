/**
 * @file
 * @author Max Godefroy
 * @date 04/08/2024.
 */

#pragma once

#include "KryneEngine/Core/Graphics/Buffer.hpp"
#include "KryneEngine/Core/Graphics/GraphicsContext.hpp"
#include "KryneEngine/Core/Graphics/Handles.hpp"
#include "KryneEngine/Core/Graphics/MemoryBarriers.hpp"

namespace KryneEngine::Modules::GraphicsUtils
{
    class DynamicBuffer
    {
    public:
        explicit DynamicBuffer(AllocatorInstance _allocator);

        void Init(GraphicsContext* _graphicsContext, const BufferCreateDesc& _bufferDesc, u8 _frameCount);
        void RequestResize(u64 _size);
        void* Map(GraphicsContext* _graphicsContext, u8 _frameIndex);
        void Unmap(GraphicsContext* _graphicsContext);
        void PrepareBuffers(
            GraphicsContext* _graphicsContext,
            CommandListHandle _commandLine,
            KryneEngine::BarrierAccessFlags _accessFlags,
            u8 _frameIndex);

        [[nodiscard]] u64 GetSize(u8 _frameIndex) const
        {
            return m_sizes[_frameIndex];
        }

        BufferHandle GetBuffer(u8 _frameIndex);

        void Destroy(GraphicsContext* _graphicsContext);

    private:
        BufferCreateDesc m_mappableRecreateDesc;
        BufferCreateDesc m_gpuRecreateDesc;
        DynamicArray<BufferHandle> m_mappableBuffers;
        DynamicArray<u64> m_sizes;
        BufferHandle m_gpuBuffer { GenPool::kInvalidHandle };
        BufferMapping m_currentMapping { { GenPool::kInvalidHandle } };

        struct BufferToFree
        {
            BufferHandle m_buffer;
            u8 m_atIndex = 0;
        };
        eastl::vector<BufferToFree> m_gpuBuffersToFree;
    };
}