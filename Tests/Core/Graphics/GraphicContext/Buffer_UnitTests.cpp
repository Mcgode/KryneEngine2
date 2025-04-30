/**
 * @file
 * @author Max Godefroy
 * @date 18/10/2024.
 */

#include "KryneEngine/Core/Graphics/Buffer.hpp"
#include "KryneEngine/Core/Graphics/GraphicsContext.hpp"
#include <gtest/gtest.h>

#include "Common.h"
#include "Utils/AssertUtils.hpp"

namespace KryneEngine::Tests::Graphics
{
    const BufferCreateDesc defaultBufferCreateDesc {
        .m_desc = {
            .m_size = 16,
            .m_debugName = "Unit Test Buffer 0",
        },
        .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::TransferDstBuffer
    };

    TEST(Buffer, CreateBuffer)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        const GraphicsCommon::ApplicationInfo appInfo = DefaultAppInfo();
        GraphicsContext* graphicsContext = GraphicsContext::Create(appInfo, nullptr, AllocatorInstance());

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        u32 errorCount = 0;

        {
            const BufferHandle buffer = graphicsContext->CreateBuffer(defaultBufferCreateDesc);
            catcher.ExpectMessageCount(errorCount);
            EXPECT_NE(buffer, GenPool::kInvalidHandle);
            EXPECT_EQ(buffer.m_handle.m_index, 0);
            EXPECT_EQ(buffer.m_handle.m_generation, 0);
        }

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        GraphicsContext::Destroy(graphicsContext);

#if defined(KE_GRAPHICS_API_VK)
        // One error from validation layers, non-destroyed buffer
        if (catcher.GetCaughtMessages().size() < errorCount)
        {
            const eastl::string& message = catcher.GetCaughtMessages()[errorCount].m_message;
            EXPECT_TRUE(
                message.find("Validation Error: [ VUID-vkDestroyDevice-device-05137 ]") != eastl::string::npos
                && message.find("name = Unit Test Buffer 0") != eastl::string::npos);
        }
        errorCount += 1;

        // One error from VMA leaks check
        if (catcher.GetCaughtMessages().size() < errorCount)
        {
            const eastl::string& message = catcher.GetCaughtMessages()[errorCount].m_message;
            EXPECT_NE(
                message.find("\"Some allocations were not freed before destruction of this memory block!\""),
                eastl::string::npos);
        }
        errorCount += 1;
#elif defined(KE_GRAPHICS_API_DX12)
        // No leak warning in DirectX 12 :(
#endif

        catcher.ExpectMessageCount(errorCount);
    }

    TEST(Buffer, DestroyBuffer)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        const GraphicsCommon::ApplicationInfo appInfo = DefaultAppInfo();
        GraphicsContext* graphicsContext = GraphicsContext::Create(appInfo, nullptr, AllocatorInstance());
        const BufferHandle buffer = graphicsContext->CreateBuffer(defaultBufferCreateDesc);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_FALSE(graphicsContext->DestroyBuffer({ GenPool::kInvalidHandle }));
        EXPECT_TRUE(graphicsContext->DestroyBuffer(buffer));
        EXPECT_FALSE(graphicsContext->DestroyBuffer(buffer));

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        GraphicsContext::Destroy(graphicsContext);
        catcher.ExpectNoMessage();
    }

    TEST(Buffer, CreateBufferOptions)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        const GraphicsCommon::ApplicationInfo appInfo = DefaultAppInfo();
        GraphicsContext* graphicsContext = GraphicsContext::Create(appInfo, nullptr, AllocatorInstance());
        eastl::vector<BufferHandle> buffers;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        u32 errorCount = 0;
        u16 index = 0;

        // Erroneous buffers
        {
            // Buffer with size 0 is invalid
            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 0, .m_debugName = "Unit test buffer 0"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::TransferDstBuffer,
            }));
            errorCount++;
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back(), GenPool::kInvalidHandle);

            // Buffer without correct usage is invalid
            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 1"},
                .m_usage = MemoryUsage::GpuOnly_UsageType,
            }));
            errorCount++;
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back(), GenPool::kInvalidHandle);
        }

        // Valid buffers with only 1 usage
        {
            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 2"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::TransferSrcBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 3"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::TransferDstBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 4"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::ConstantBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 5"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::ReadBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 6"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::WriteBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 7"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::IndexBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 8"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::VertexBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 9"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::IndirectBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            // Raytracing support currently not tracked
            const bool raytracingSupported = false;
            if (raytracingSupported)
            {
                buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                    .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 10"},
                    .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::AccelerationStruct,
                }));
                catcher.ExpectMessageCount(errorCount);
                EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));
            }
        }

        // Some multi-usage buffers
        {
            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 11"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::ReadWriteBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 12"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::IndexBuffer | MemoryUsage::VertexBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 13"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::IndirectBuffer | MemoryUsage::WriteBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 14"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::IndirectBuffer | MemoryUsage::ReadWriteBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 15"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::ConstantBuffer | MemoryUsage::TransferDstBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 16"},
                .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::TransferSrcBuffer | MemoryUsage::ReadWriteBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));
        }

        // Non GPU-only buffers
        {
            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 17"},
                .m_usage = MemoryUsage::StageOnce_UsageType | MemoryUsage::TransferSrcBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 18"},
                .m_usage = MemoryUsage::StageEveryFrame_UsageType | MemoryUsage::TransferSrcBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 19"},
                .m_usage = MemoryUsage::StageEveryFrame_UsageType | MemoryUsage::ConstantBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));

            buffers.push_back(graphicsContext->CreateBuffer(BufferCreateDesc{
                .m_desc = {.m_size = 16, .m_debugName = "Unit test buffer 20"},
                .m_usage = MemoryUsage::Readback_UsageType | MemoryUsage::TransferDstBuffer,
            }));
            catcher.ExpectMessageCount(errorCount);
            EXPECT_EQ(buffers.back().m_handle, (GenPool::Handle{index++}));
        }

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        for (const BufferHandle& handle: buffers)
        {
            graphicsContext->DestroyBuffer(handle);
        }
        GraphicsContext::Destroy(graphicsContext);
        catcher.ExpectMessageCount(errorCount);
    }
}
