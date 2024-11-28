/**
 * @file
 * @author Max Godefroy
 * @date 27/11/2024.
 */

#include <EASTL/initializer_list.h>
#include <gtest/gtest.h>
#include <KryneEngine/Core/Graphics/Common/Buffer.hpp>
#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>
#include <KryneEngine/Core/Graphics/Common/MemoryBarriers.hpp>

#include "Common.h"
#include "Utils/AssertUtils.hpp"

namespace KryneEngine::Tests::Graphics
{
    TEST(ResourceCopy, StagingBufferCopy)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        const GraphicsCommon::ApplicationInfo appInfo = DefaultAppInfo();
        GraphicsContext* graphicsContext = GraphicsContext::Create(appInfo, nullptr);

        constexpr size_t payload = 0x0123456789abcdef;

        BufferHandle srcBuffer = graphicsContext->CreateBuffer({
            .m_desc = {
                .m_size = sizeof(payload),
                .m_debugName = "SrcBuffer",
            },
            .m_usage = MemoryUsage::StageOnce_UsageType | MemoryUsage::TransferSrcBuffer,
        });

        BufferHandle dstBuffer = graphicsContext->CreateBuffer({
            .m_desc = {
                .m_size = sizeof(payload),
                .m_debugName = "DstBuffer",
            },
            .m_usage = MemoryUsage::StageOnce_UsageType | MemoryUsage::TransferDstBuffer,
        });


        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        CommandListHandle commandList = graphicsContext->BeginGraphicsCommandList();

        {
            BufferMapping srcMapping { srcBuffer, sizeof(payload) };
            graphicsContext->MapBuffer(srcMapping);
            memcpy(srcMapping.m_ptr, &payload, sizeof(payload));
            graphicsContext->UnmapBuffer(srcMapping);
        }

        BufferMemoryBarrier barriers[] = {
            BufferMemoryBarrier {
                .m_stagesSrc = BarrierSyncStageFlags::All,
                .m_stagesDst = BarrierSyncStageFlags::Transfer,
                .m_accessSrc = BarrierAccessFlags::None,
                .m_accessDst = BarrierAccessFlags::TransferSrc,
                .m_buffer = srcBuffer,
            },
            BufferMemoryBarrier {
                .m_stagesSrc = BarrierSyncStageFlags::All,
                .m_stagesDst = BarrierSyncStageFlags::Transfer,
                .m_accessSrc = BarrierAccessFlags::None,
                .m_accessDst = BarrierAccessFlags::TransferDst,
                .m_buffer = dstBuffer,
            },
        };

        graphicsContext->PlaceMemoryBarriers(
            commandList,
            {},
            { barriers },
            {});

        graphicsContext->CopyBuffer(
            commandList,
            {
                .m_copySize = sizeof(payload),
                .m_bufferSrc = srcBuffer,
                .m_bufferDst = dstBuffer,
            });

        graphicsContext->EndGraphicsCommandList();
        graphicsContext->EndFrame();
        graphicsContext->WaitForLastFrame();

        {
            BufferMapping dstMapping { dstBuffer, sizeof(payload), 0, false };
            graphicsContext->MapBuffer(dstMapping);
            const size_t result = *static_cast<size_t*>(dstMapping.m_ptr);
            EXPECT_EQ(result, payload);
            graphicsContext->UnmapBuffer(dstMapping);
        }

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        graphicsContext->DestroyBuffer(dstBuffer);
        graphicsContext->DestroyBuffer(srcBuffer);

        GraphicsContext::Destroy(graphicsContext);
        catcher.ExpectNoMessage();
    }
}