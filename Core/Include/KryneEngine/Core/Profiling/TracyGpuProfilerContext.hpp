/**
 * @file
 * @author Max Godefroy
 * @date 21/09/2025.
 */

#pragma once

#include <EASTL/span.h>

#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"
#include "KryneEngine/Core/Memory/DynamicArray.hpp"
#include "KryneEngine/Core/Threads/SpinLock.hpp"

namespace KryneEngine
{
    class GraphicsContext;

    class TracyGpuProfilerContext
    {
    public:
        TracyGpuProfilerContext(AllocatorInstance _allocator, u32 _frameContextCount);
        ~TracyGpuProfilerContext();

        u16 ReserveQuery();

        void SetQueryTimestampIndex(u16 _queryIndex, u32 _timestampIndex);

        void EndFrame(u64 _frameId);

        void ResolveQueries(const GraphicsContext* _graphicsContext, u64 _frameId);

        [[nodiscard]] u8 GetContextId() const { return m_tracyContextId; }

    private:
        AllocatorInstance m_allocator;
        SpinLock m_queryRingBufferLock;
        u32 m_queryRingBufferHead = 0;
        u32 m_queryRingBufferTail = 0;
        u32* m_queryRingBuffer = nullptr;
        DynamicArray<eastl::pair<u32, u32>> m_frameContextQueryRanges;
        u8 m_tracyContextId = 0;

        static constexpr size_t kQueryRingBufferCapacity = 1 << 16;
    };
}
